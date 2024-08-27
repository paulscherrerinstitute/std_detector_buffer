import json

import cv2
import numpy as np
import zmq
from fastapi import FastAPI, Response

app = FastAPI()


class ImageReceiver:
    def __init__(self, address, mode=zmq.SUB):
        self.context = zmq.Context()
        self.socket = self.context.socket(mode)
        self.socket.connect(address)
        if mode == zmq.SUB:
            self.socket.subscribe("")
        self.running = True
        self.header = None
        self.shape = None
        self.dtype = None
        self.data = None

    def get_image(self):
        try:
            while self.running:
                header = self.socket.recv(zmq.NOBLOCK)
                if header is not None:
                    try:
                        # Decode header
                        self.header = json.loads(header.decode("utf-8"))
                        self.shape = self.header.get("shape", None)
                        self.dtype = self.header.get("type", "int8")

                        # Receive and decode image data
                        data = self.socket.recv()
                        image_array = np.frombuffer(data, dtype=self.dtype).reshape(
                            self.shape
                        )

                        # Convert the image array to a JPEG image in memory
                        success, jpg_image = cv2.imencode(".jpg", image_array)
                        if not success:
                            raise ValueError("Failed to encode image to JPEG")

                        # Return the JPEG image as bytes
                        return jpg_image.tobytes()

                    except Exception as e:
                        print(f"Error processing image data: {e}")
                        continue
        except zmq.Again:
            pass
        except Exception as e:
            print(f"Unexpected error: {e}")
            self.running = False
