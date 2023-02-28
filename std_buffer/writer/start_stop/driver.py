from threading import Thread, Event, Lock
from google.protobuf.json_format import MessageToDict
from std_daq.image_metadata_pb2 import ImageMetadata
from std_daq.writer_command_pb2 import WriterCommand, CommandType, RunInfo, StatusReport
from time import time, sleep, time_ns
import logging
import zmq

_logger = logging.getLogger(__name__)

RECV_TIMEOUT_MS = 500
STATS_INTERVAL_TIME = 0.5

class WriterStatusTracker(object):
    EMPTY_STATS = {"n_write_completed": 0, "n_write_requested": 0, "start_time": None, "stop_time": None}

    def __init__(self, ctx, in_status_address, out_status_address, stop_event):
        self.ctx = ctx
        
        self.status = {'state': 'READY', 'acquisition': {'state': "FINISHED", 'info': None, 'stats': None}}
        self.status_lock = Lock()

        self.last_status_send_time = 0
        self._current_run_id = None

        self.status_receiver = self.ctx.socket(zmq.PULL)
        self.status_receiver.RCVTIMEO = RECV_TIMEOUT_MS
        self.status_receiver.bind(in_status_address)

        self.status_sender = self.ctx.socket(zmq.PUB)
        self.status_sender.bind(out_status_address)

        self.stop_event = stop_event
        self.status_thread = Thread(target=self._status_rcv_thread)
        self.status_thread.start()

    def get_status(self):
        with self.status_lock:
            return self.status

    def _status_rcv_thread(self):
        status_message = StatusReport()

        while not self.stop_event.is_set():
            try:
                status = self.status_receiver.recv()
                status_message.ParseFromString(status)

                status_run_id = status_message.run_info.run_id

                if status_message.command_type == CommandType.START_WRITING:
                    self._log_start_status(status_run_id, status_message)
                    return
                   
                if status_message.command_type == CommandType.STOP_WRITING:
                    self._log_stop_status(status_message)
                    return

                if status_run_id != self._current_run_id:
                    error_message = f"Received write status for run_id={status_run_id} while the " \
                        "_current_run_id={self._current_run_id}. Status={MessageToDict(status_message)}" 

                    raise ValueError(error_message)

                if status_message.command_type == CommandType.WRITE_IMAGE:
                    self._log_write_status(status_message)
                    return

                _logger.warning(f"Unknown status message received: {status_message}")
            except zmq.Again:
                pass
            except ValueError:
                _logger.exception()

    def _log_write_status(self, status):
        with self.status_lock:
            self.status['state'] = 'WRITING'
            self.status['acquisition']['state'] = 'ACQUIRING_IMAGES'
            self.status['acquisition']['stats']['n_write_completed'] += 1

        # Send out write progress updates on STATS_INTERVAL_TIME intervals.
        current_time = time()
        if current_time - self.last_status_send_time > STATS_INTERVAL_TIME:
            self.last_status_send_time = current_time    
            self.status_sender.send_json(self.status)

    def _log_start_status(self, run_id, status):
        with self.status_lock:
            self.status = {'state': "WRITING", 
                           'acquisition': {'state': 'WAITING_FOR_IMAGES',
                                           'stats': dict(self.EMPTY_STATS),
                                           'info': {'output_file': status.run_info.output_file,
                                                    'n_images': status.run_info.n_images,
                                                    'run_id': run_id}}}
            self.status['acquisition']['stats']['start_time'] = time()

        self.last_status_send_time = 0
        self._current_run_id = run_id

        # Send status update as soon as writer starts.
        self.status_sender.send_json(self.status)

    def _log_stop_status(self, status):
        with self.status_lock:
            self.status['state'] = 'READY'
            self.status['acquisition']['state'] = 'FINISHED'
            self.status['acquisition']['stats']['stop_time'] = time()

        self._current_run_id = None

        # Send status update as soon as writer stops.
        self.status_sender.send_json(self.status)

    def log_write_request(self, run_id, image_id):
        if run_id != self._current_run_id:
            _logger.debug(f"Received write_request for image_id={image_id}, run_id={run_id} while the _current_run_id={self._current_run_id}.")
            return

        with self.status_lock:
            self.status['acquisition']['stats']['n_write_requested'] += 1


class WriterDriver(object):

    WRITER_DRIVER_IPC_ADDRESS = 'inproc://WriterDriverCommand'
    POLLER_TIMEOUT_MS = 1000
    # In seconds.
    STARTUP_WAIT_TIME = 200/1000

    START_COMMAND = 'START'
    STOP_COMMAND = 'STOP'

    def __init__(self, ctx, command_address, in_status_address, out_status_address, image_metadata_address):
        self.ctx = ctx
        self.stop_event = Event()
        self.status = WriterStatusTracker(ctx, in_status_address, out_status_address, self.stop_event)
        _logger.info(f'Starting writer driver with command_address:{command_address} \
                                                   in_status_address:{in_status_address} \
                                                   out_status_address:{out_status_address} \
                                                   image_metadata_address:{image_metadata_address}')

        self.user_command_sender = self.ctx.socket(zmq.PUSH)
        self.user_command_sender.bind(self.WRITER_DRIVER_IPC_ADDRESS)

        self.processing_t = Thread(target=self.processing_thread, args=(command_address, image_metadata_address))
        self.processing_t.start()

    def get_status(self):
        return self.status.get_status()

    def start(self, run_info):
        self._send_command(self.START_COMMAND, run_info)

    def stop(self):
        self._send_command(self.STOP_COMMAND)

    def close(self):
        _logger.info(f'Closing writer driver.')
        self.stop_event.set()
        self.processing_t.join()

    def _send_command(self, command, run_info=None):
        if run_info is None:
            run_info = {}

        command = {'COMMAND': command, 'run_info': run_info}
        _logger.info(f"Execute command in driver {command}'.")

        self.user_command_sender.send_json(command)

    def processing_thread(self, command_address, image_metadata_address):
        user_command_receiver = self.ctx.socket(zmq.PULL)
        user_command_receiver.connect(self.WRITER_DRIVER_IPC_ADDRESS)

        writer_command_sender = self.ctx.socket(zmq.PUB)
        writer_command_sender.bind(command_address)

        image_metadata_receiver = self.ctx.socket(zmq.SUB)
        image_metadata_receiver.connect(image_metadata_address)
        
        # Wait for connections to happen.
        sleep(self.STARTUP_WAIT_TIME)

        image_meta = ImageMetadata()
        writer_command = WriterCommand()

        def process_start_command(run_info):
            # Use current time as run_id.
            run_info['run_id'] = time_ns()
            
            # Tell the writer to start writing.
            nonlocal writer_command
            writer_command.command_type = CommandType.START_WRITING
            writer_command.run_info.CopyFrom(RunInfo(**run_info))
            writer_command.metadata.Clear()

            _logger.info(f"Send start command to writer: {MessageToDict(writer_command)}.")
            writer_command_sender.send(writer_command.SerializeToString())
            nonlocal i_image
            i_image = 0

            while True:
                status = self.status.get_status()
                if status['state'] == "WRITING":
                    break
                sleep(0.1)

            # Subscribe to the ImageMetadata stream.
            image_metadata_receiver.setsockopt(zmq.SUBSCRIBE, b'')

        def process_stop_command():
            # Stop listening to new image metadata.
            image_metadata_receiver.setsockopt(zmq.UNSUBSCRIBE, b'')

            # Drain image_metadata received so far.
            try:
                image_metadata_receiver.recv(flags=zmq.NOBLOCK)
            except zmq.Again:
                pass

            nonlocal writer_command
            writer_command.command_type = CommandType.STOP_WRITING
            writer_command.metadata.Clear()

            _logger.info(f"Send stop command to writer: {MessageToDict(writer_command)}.")
            writer_command_sender.send(writer_command.SerializeToString())

        poller = zmq.Poller()
        poller.register(user_command_receiver, zmq.POLLIN)
        poller.register(image_metadata_receiver, zmq.POLLIN)

        i_image = 0

        while not self.stop_event.is_set():
            try:

                events = dict(poller.poll(timeout=RECV_TIMEOUT_MS))
                
                if user_command_receiver in events:
                    command = user_command_receiver.recv_json(flags=zmq.NOBLOCK)

                    if command['COMMAND'] == self.START_COMMAND:
                        process_start_command(command['run_info'])
                    elif command['COMMAND'] == self.STOP_COMMAND:
                        process_stop_command()
                    else:
                        _logger.warning(f"Unknown command:{command}.")

                if image_metadata_receiver in events:
                    meta_raw = image_metadata_receiver.recv(flags=zmq.NOBLOCK)
                    image_meta.ParseFromString(meta_raw)
                    writer_command.metadata.CopyFrom(image_meta)
                    writer_command.command_type = CommandType.WRITE_IMAGE
                    writer_command.i_image = i_image
                
                    self.status.log_write_request(writer_command.run_info.run_id, image_meta.image_id)
                    writer_command_sender.send(writer_command.SerializeToString())

                    # Terminate writing.
                    i_image += 1
                    if i_image == writer_command.run_info.n_images:
                        process_stop_command()
            except Exception as e:
                print(f"Error in driver loop: {str(e)}")

