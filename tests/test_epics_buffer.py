import json
import random
import unittest

import zmq
from epics import CAProcess
from pcaspy import Driver, SimpleServer
from std_daq_service.epics_buffer.receiver import EpicsReceiver
from std_daq_service.epics_buffer.start_receiver import start_epics_buffer


class TestEpicsBuffer(unittest.TestCase):
    def test_events(self):
        pv_names = ["ioc:pv_1", "ioc:pv_2"]

        buffer = []

        def callback_function(**kwargs):
            buffer.append(kwargs)

        EpicsReceiver(pv_names, callback_function)

        class TestIoc(Driver):
            prefix = "ioc:"
            pvdb = {
                "pv_1": {"scan": 1},
                "pv_2": {"scan": 2}
            }

            def __init__(self):
                Driver.__init__(self)

            def read(self, reason):
                value = random.random()
                return value

        ioc_server = SimpleServer()
        ioc_server.createPV(TestIoc.prefix, TestIoc.pvdb)
        driver = TestIoc()

        while len(buffer) < 5:
            ioc_server.process(0.1)

        # First 2 records come as connection updates.
        self.assertEqual({buffer[0]['pv_name'], buffer[1]['pv_name']}, set(pv_names))
        self.assertEqual(buffer[0]["value"], None)
        self.assertEqual(buffer[1]["value"], None)

        # We should get at least 1 value update from each PV.
        pv_updates = set()
        for i in range(2, 5):
            pv_updates.add(buffer[i]["pv_name"])
            self.assertTrue(buffer[i]['value'] is not None)
        self.assertEqual(pv_updates, set(pv_names))

        ioc_server.cl

    def test_receiver(self):
        sampling_pv = 'ioc:pulse_id'
        pv_names = ['ioc:pv_1', 'ioc:pv_2']
        stream_url = 'tcp://127.0.0.1:7000'
        recv_process = CAProcess(target=start_epics_buffer, args=(sampling_pv, pv_names, stream_url))
        recv_process.start()

        class TestIoc(Driver):
            prefix = "ioc:"
            pvdb = {
                "pulse_id": {"scan": 0.1},
                "pv_1": {"scan": 1},
                "pv_2": {"scan": 2}
            }

            def __init__(self):
                Driver.__init__(self)
                self.pulse_id = 0

            def read(self, reason):
                if reason == "pulse_id":
                    self.pulse_id += 1
                    return self.pulse_id

                value = random.random()
                return value

        ioc_server = SimpleServer()
        ioc_server.createPV(TestIoc.prefix, TestIoc.pvdb)
        driver = TestIoc()

        ctx = zmq.Context()
        receiver = ctx.socket(zmq.SUB)
        receiver.setsockopt(zmq.RCVTIMEO, 100)
        receiver.connect(stream_url)
        receiver.setsockopt_string(zmq.SUBSCRIBE, "")

        recv_buffer = []

        while len(recv_buffer) <= 20:
            try:
                pulse_id_bytes, data = receiver.recv_multipart(flags=zmq.NOBLOCK)
                recv_buffer.append((int.from_bytes(pulse_id_bytes, 'little'), data))
            except zmq.Again:
                pass

            ioc_server.process(0.01)

        receiver.close()
        recv_process.terminate()

        for i in range(20):
            recv_pulse_id = recv_buffer[i][0]
            recv_data = json.loads(recv_buffer[i][1])

            self.assertEqual(recv_pulse_id, i)
            # self.assertEqual(recv_data["ioc:pv_1"]["event_timestamp"])
            print(recv_data)