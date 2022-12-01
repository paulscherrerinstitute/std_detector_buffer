from threading import Thread
import zmq


class WriterStatusTracker(object):
    RECV_TIMEOUT_MS = 500
    STATUS_READY = {'state': "READY"}

    def __init__(self, ctx, status_address):
        self.ctx = ctx
        self.status = self.STATUS_READY
        self.processing_thread = Thread(target=self._processing_thread, args=(status_address,))

    def get_status(self):
        return self.status

    def _processing_thread(self, status_address):
        writer_status_receiver = self.ctx.socket(zmq.PULL)
        writer_status_receiver.RCVTIMEO = self.RECV_TIMEOUT_MS
        writer_status_receiver.bind(status_address)

        while True:
            try:
                status = writer_status_receiver.recv()
                self._process_received_status(status)
            except zmq.Again:
                pass

    def _process_received_status(self, status):
        pass

    def sent_image_write_request(self, image_id):
        pass


class WriterDriver(object):

    WRITER_DRIVER_IPC_ADDRESS = 'inproc://WriterDriverCommand'
    POLLER_TIMEOUT_MS = 1000

    WRITE_COMMAND = 'WRITE'
    STOP_COMMAND = 'STOP'

    def __init__(self, ctx, command_address, status_address, image_metadata_address):
        self.ctx = ctx
        self.status = WriterStatusTracker(ctx, status_address)

        self.user_command_sender = self.ctx.socket(zmq.PUSH)
        self.user_command_sender.connect(self.WRITER_DRIVER_IPC_ADDRESS)

        self.processing_t = Thread(target=self.processing_thread, args=(command_address, image_metadata_address))
        self.processing_t.start()

    def get_status(self):
        return self.status.get_status()

    def send_command(self, command, data=None):
        if data is None:
            data = {}
        data['COMMAND'] = command

        self.user_command_sender.send_json(command)

    def processing_thread(self, command_address, image_metadata_address):
        user_command_receiver = self.ctx.socket(zmq.PULL)
        user_command_receiver.connect(self.WRITER_DRIVER_IPC_ADDRESS)

        writer_command_sender = self.ctx.socket(zmq.PUB)
        writer_command_sender.bind(command_address)

        image_metadata_receiver = self.ctx.socket(zmq.SUB)
        image_metadata_receiver.connect(image_metadata_address)

        def process_start_command(command):
            # Tell the writer to start writing.
            start_command = command
            writer_command_sender.send(start_command)

            # Subscribe to the ImageMetadata stream.
            image_metadata_receiver.setsockopt(zmq.SUBSCRIBE, '')

        def process_stop_command():
            # Stop listening to new image metadata.
            image_metadata_receiver.setsockopt(zmq.UNSUBSCRIBE, '')

            # Drain image_metadata received so far.
            try:
                image_metadata_receiver.recv(flags=zmq.NOBLOCK)
            except zmq.Again:
                pass

            # Tell the writer to stop writing.
            stop_command = None
            writer_command_sender.send(stop_command)

        while True:
            # Check if the user made any requests.
            try:
                command = user_command_receiver.recv_json(flags=zmq.NOBLOCK)
                if command['COMMAND'] == self.WRITE_COMMAND:
                    process_start_command(command)
                elif command['COMMAND'] == self.STOP_COMMAND:
                    process_stop_command()
            except zmq.Again:
                pass

            try:
                # Drain the image_metadata socket.
                while True:
                    image_meta = image_metadata_receiver.recv(flags=zmq.NOBLOCK)

                    write_image_command = None
                    self.status.sent_image_write_request(image_meta.image_id)
                    writer_command_sender.send(write_image_command)
            except zmq.Again:
                pass
