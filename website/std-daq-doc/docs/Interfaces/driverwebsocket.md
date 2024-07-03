---
sidebar_position: 5
id: websocket
title: Driver Websocket
---

## Driver WebSocket Interface for controlling Writers

This API facilitates the control of a driver via WebSocket communication, providing interfaces to start and stop processes related to image file writing.

### API Commands
#### General Format

Commands are JSON formatted and should be sent through the established WebSocket connection.

1. Start Command

    Starts the image writing process to a specified directory.

    ```json
    {
      "command": "start",
      "path": "<file_path>",
      "n_image": <value>,
      "writer_id": <value>,
      "start_id": <value>
    }
    ```
    Parameters:
    - `path` (string, mandatory): The filesystem path where image files will be created. The specified directory must pre-exist.
    - `n_image` (integer, optional): The number of images to save. Default is `16777215` (maximum for Gigafrost in gfclient).
    - `writer_id` (integer, optional): ID of the user under whose account files are written. Default is `0` (root). Other IDs have not been tested.
    - `start_id` (integer, optional): Image ID from which to start writing. Default is `0`.

    ```json
    {"command": "start", "path": "/gpfs/test/test-beamline", "n_image": 100000}
    ```
2. Stop Command

    Stops the ongoing process.

    ```json
    {"command": "stop"}
    ```
### Communication Feedback

The driver will send updates every 1 second or upon any event change via the same WebSocket connection. Communication will continue until all files are saved or an error occurs, with the last message indicating either an error or successful file save.

### Connection Termination

The WebSocket connection will be automatically closed either upon the completion of file saving or if an error interrupts the process.
