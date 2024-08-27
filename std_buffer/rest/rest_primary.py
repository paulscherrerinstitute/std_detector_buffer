import argparse
import hashlib
import json
import logging
import os
from time import time



import requests
import zmq
from fastapi import Depends, FastAPI, HTTPException, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from jsonschema import ValidationError, validate
from stats_logger import StatsLogger
from utils import (
    EventFilter,
    create_interleaved_vds,
    get_dataset_details,
    print_dataset_details,
    read_metadata,
)
from image import ImageReceiver
from uvicorn import run

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(event)s %(levelname)s: %(message)s",
    handlers=[logging.StreamHandler()],
)
logger = logging.getLogger(__name__)
logger.addFilter(EventFilter())

# JSON Schema for validation
JSON_SCHEMA = {
    "properties": {
        "detector_name": {"type": "string"},
        "detector_type": {"type": "string"},
        "n_modules": {"type": "integer"},
        "bit_depth": {"type": "integer"},
        "image_pixel_height": {"type": "integer"},
        "image_pixel_width": {"type": "integer"},
        "start_udp_port": {"type": "integer"},
        "writer_user_id": {"type": "integer"},
        "submodule_info": {"type": "object"},
        "max_number_of_forwarders_spawned": {"type": "integer"},
        "use_all_forwarders": {"type": "boolean"},
        "module_sync_queue_size": {"type": "integer"},
        "module_positions": {"type": "object"},
    },
}

# FastAPI app initialization
app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize ZMQ context (optional)
ctx = zmq.Context()
stats_logger = StatsLogger(ctx)

# Global variables to be set at startup
config_file = None
secondary_server = None


# Dependency functions
def get_config_file():
    return config_file


def get_secondary_server():
    return secondary_server


# FastAPI endpoints
@app.get("/api/config/get")
async def get_configuration(user: str):
    start_time = time()
    config_file = get_config_file()
    logger.info(f"User {user} fetching configuration from {config_file}")
    try:
        with open(config_file, "r") as file:
            config = json.load(file)
        duration = time() - start_time
        logger.info(
            f"User {user} successfully fetched configuration in {duration:.2f} seconds"
        )
        stats_logger.log_config_change("get", user, True)
        return config
    except FileNotFoundError:
        logger.error(f"Configuration file not found: {config_file}")
        stats_logger.log_config_change("get", user, False)
        raise HTTPException(status_code=404, detail="Configuration file not found")
    except json.JSONDecodeError:
        logger.error(f"Error decoding JSON file: {config_file}")
        stats_logger.log_config_change("get", user, False)
        raise HTTPException(status_code=500, detail="Error decoding JSON file")


@app.post("/api/config/set")
async def update_configuration(
    request: Request,
    user: str,
):
    start_time = time()
    new_config = await request.json()
    config_file = get_config_file()
    existing_config = None

    logger.info(f"Fetching existing configuration from {config_file}")
    try:
        with open(config_file, "r") as file:
            existing_config = json.load(file)
    except FileNotFoundError:
        logger.error(f"Configuration file not found: {config_file}")
        raise HTTPException(status_code=404, detail="Configuration file not found")
    except json.JSONDecodeError:
        logger.error(f"Error decoding JSON file: {config_file}")
        raise HTTPException(status_code=500, detail="Error decoding JSON file")

    # Block if detector name or type are being altered
    if (
        new_config.get("detector_name") != existing_config.get("detector_name")
        or new_config.get("detector_type") != existing_config.get("detector_type")
    ) {
        logger.error("Detector name or detector type cannot be changed.")
        raise HTTPException(
            status_code=400, detail="Detector name or detector type cannot be changed."
        )
    

    logger.info(f"User {user} received new configuration: {new_config}")
    try:
        validate(instance=new_config, schema=JSON_SCHEMA)
    except ValidationError as e:
        logger.error(f"Validation error: {e}")
        stats_logger.log_config_change("set", user, False)
        raise HTTPException(status_code=400, detail=str(e))

    try:
        # Overwrite the local file
        logger.info("Schema is validated.")
        with open(config_file, "w") as file:
            json.dump(new_config, file, indent=4)
        duration = time() - start_time
        logger.info(
            f"User {user} successfully updated configuration in {duration:.2f} seconds"
        )
        stats_logger.log_config_change("set", user, True)

        # Sends it to the secondary server
        secondary_server = get_secondary_server()
        if secondary_server is not None:
            response = requests.post(
                f"{secondary_server}/api/config/set",
                params={"user": user},
                json=new_config,
                headers={"Content-Type": "application/json"},
            )
            if response.status_code != 200:
                raise HTTPException(
                    status_code=500,
                    detail="Failed to synchronize with the secondary server",
                )
            return {"message": "Configuration updated and synchronized successfully"}
    except Exception as e:
        logger.error(f"Error writing to configuration file: {e}")
        stats_logger.log_config_change("set", user, False)
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/live/get_image")
async def get_image():
    # std-daq live stream coming 
    image_receiver = ImageReceiver("tcp://129.129.95.38:20001")
    try:
        jpg_image = image_receiver.get_image()
        if jpg_image is not None:
            return Response(content=jpg_image, media_type="image/jpeg")
        else:
            return Response(content="No image data received", status_code=204)
    except Exception as e:
        return Response(content=f"Error: {str(e)}", status_code=500)


@app.get("/api/h5/read_metadata")
async def read_metadata_endpoint(filename: str):
    try:
        metadata = read_metadata(filename)
        return JSONResponse(content=metadata)
    except Exception as e:
        logger.error(f"Error reading metadata from {filename}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/api/h5/get_metadata_status")
async def read_metadata_status(filename: str):
    try:
        details = await get_dataset_details(filename)
        return JSONResponse(content=details)
    except Exception as e:
        logger.error(f"Error getting dataset details from {filename}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/api/h5/create_interleaved_vds")
async def create_interleaved_vds_endpoint(request: Request):
    try:
        payload = await request.json()
        base_path = payload.get("base_path")
        output_file = payload.get("output_file")

        num_files = len(
            [
                f
                for f in os.listdir(base_path)
                if f.endswith(".h5")
                and f.startswith("file")
                and os.path.isfile(os.path.join(base_path, f))
            ]
        )
        logger.info(
            f"Received request to create interleaved VDS with base_path: {base_path}, num_files: {num_files}, output_file: {output_file}"
        )

        if not base_path or not num_files or not output_file:
            raise ValueError("Missing required parameters")

        logger.info(f"Number of files in base_path: {num_files}")
        logger.info("Starting the VDS creation process...")
        create_interleaved_vds(base_path, num_files, output_file)
        logger.info("VDS creation process completed successfully.")
        return {"message": "Interleaved virtual dataset created successfully"}
    except Exception as e:
        logger.error(f"Error creating interleaved virtual dataset: {e}")
        raise HTTPException(status_code=500, detail=str(e))


def start_api(config_file_path, rest_port, secondary_server_address):
    global config_file, secondary_server
    config_file = config_file_path
    secondary_server = secondary_server_address
    
    try:
        if secondary_server not None:
            logger.info(
                f"Starting API with config file: {config_file} on port {rest_port} and with secondary server: {secondary_server} "
            )
        else:
        logger.info(
                f"Starting API with config file: {config_file} on port {rest_port} and without secondary server."
            )
        
        run(app, host="0.0.0.0", port=rest_port, log_level="warning")
    except Exception as e:
        logger.exception("Error while trying to run the REST api")

def initialize_globals(config_file_path, secondary_server_address):
    global config_file, secondary_server
    config_file = config_file_path
    secondary_server = secondary_server_address

def main():
    parser = argparse.ArgumentParser(
        description="Standard DAQ Start Stop REST interface"
    )
    parser.add_argument("config_file", type=str, help="Path to JSON config file.")
    parser.add_argument("--rest_port", type=int, help="Port for REST api", default=5000)
    parser.add_argument(
        "--secondary_server",
        type=str,
        default=None,
        help="Address of the secondary server for synchronization",
    )

    args = parser.parse_args()

    initialize_globals(config_file, secondary_server)

    start_api(
        config_file_path=args.config_file,
        rest_port=args.rest_port,
        secondary_server_address=args.secondary_server,
    )


if __name__ == "__main__":
    main()
