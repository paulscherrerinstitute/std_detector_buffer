from threading import Thread, Event
from std_daq.image_metadata_pb2 import ImageMetadata
from std_daq.writer_command_pb2 import WriterCommand, CommandType, RunInfo 
from time import time
import logging
import zmq

_logger = logging.getLogger(__name__)

RECV_TIMEOUT_MS = 500

class WriterStatusTracker(object):
    STATUS_READY = {'state': "READY"}
    STATUS_WRITING = {'state': "WRITING"}

    def __init__(self, ctx, status_address, stop_event):
        self.ctx = ctx
        self.stop_event = stop_event
        self.status = self.STATUS_READY
        self.processing_thread = Thread(target=self._processing_thread, args=(status_address,))

        # Keep track of the open write requests.
        self._open_write_requests = {}

    def get_status(self):
        return self.status

    def _processing_thread(self, status_address):
        writer_status_receiver = self.ctx.socket(zmq.PULL)
        writer_status_receiver.RCVTIMEO = RECV_TIMEOUT_MS
        writer_status_receiver.bind(status_address)

        while not self.stop_event.is_set():
            try:
                status = writer_status_receiver.recv()
                self._process_received_status(status)
            except zmq.Again:
                pass

    def _process_received_status(self, status):
        print(f'Received status {status}')

    def log_start_request(self, run_info):
        _logger.debug(f"Sent start request with run_info {run_info}.")
        self.status = self.STATUS_WRITING | {'start_request_time': time(), 'run_info': run_info}
        self._open_write_requests = {}

    def log_stop_request(self):
        _logger.debug("Sent stop request")
        self.status['stop_request_time'] = time()

    def log_write_request(self, image_id):
        _logger.debug(f"Sent write request for image_id {image_id}.")
        self._open_write_requests[image_id] = time()


class WriterDriver(object):

    WRITER_DRIVER_IPC_ADDRESS = 'inproc://WriterDriverCommand'
    POLLER_TIMEOUT_MS = 1000

    WRITE_COMMAND = 'WRITE'
    STOP_COMMAND = 'STOP'

    def __init__(self, ctx, command_address, status_address, image_metadata_address):
        self.ctx = ctx
        self.stop_event = Event()
        self.status = WriterStatusTracker(ctx, status_address, self.stop_event)
        _logger.info(f'Starting writer driver with command_address:{command_address} \
                                                   status_address:{status_address} \
                                                   image_metadata_address:{image_metadata_address}')

        self.user_command_sender = self.ctx.socket(zmq.PUSH)
        self.user_command_sender.bind(self.WRITER_DRIVER_IPC_ADDRESS)

        self.processing_t = Thread(target=self.processing_thread, args=(command_address, image_metadata_address))
        self.processing_t.start()

    def get_status(self):
        return self.status.get_status()

    def start(self, run_info):
        self._send_command(self.WRITE_COMMAND, run_info)

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

        image_meta = ImageMetadata()
        writer_command = WriterCommand()

        def process_start_command(run_info):
            # Tell the writer to start writing.
            nonlocal writer_command
            writer_command.command_type = CommandType.START_WRITING
            writer_command.run_info.CopyFrom(RunInfo(**run_info))
            writer_command.metadata.Clear()

            _logger.info(f"Send start command to writer: {writer_command}.")
            writer_command_sender.send(writer_command.SerializeToString())
            self.status.log_start_request(run_info)

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

            _logger.info(f"Send stop command to writer: {writer_command}.")
            writer_command_sender.send(writer_command.SerializeToString())
            self.status.log_stop_request()

        poller = zmq.Poller()
        poller.register(user_command_receiver, zmq.POLLIN)
        poller.register(image_metadata_receiver, zmq.POLLIN)

        while not self.stop_event.is_set():
            events = dict(poller.poll(timeout=RECV_TIMEOUT_MS))
            
            if user_command_receiver in events:
                command = user_command_receiver.recv_json(flags=zmq.NOBLOCK)

                if command['COMMAND'] == self.WRITE_COMMAND:
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
            
                self.status.log_write_request(image_meta.image_id)
                writer_command_sender.send(writer_command.SerializeToString())
