import argparse
import hashlib
import json
import logging
from time import time

import requests
import zmq
from fastapi import Depends, FastAPI, HTTPException, Request
from fastapi.middleware.cors import CORSMiddleware
from jsonschema import ValidationError, validate
from stats_logger import StatsLogger
from utils import (
    EventFilter,
    create_interleaved_vds,
    print_dataset_details,
    read_metadata,
)
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
    global config_file
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
    global config_file
    logger.info(f"User {user} received new configuration: {new_config}")
    try:
        validate(instance=new_config, schema=JSON_SCHEMA)
    except ValidationError as e:
        logger.error(f"Validation error: {e}")
        stats_logger.log_config_change("set", user, False)
        raise HTTPException(status_code=400, detail=str(e))

    try:
        # Overwrite the local file
        logger.info("schema is validated.")
        with open(config_file, "w") as file:
            json.dump(new_config, file, indent=4)
        duration = time() - start_time
        logger.info(
            f"User {user} successfully updated configuration in {duration:.2f} seconds"
        )
        stats_logger.log_config_change("set", user, True)

        # Generate hash for the configuration
        response = requests.post(
            f"{secondary_server}/api/config/set",
            params={"user":user},
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


@app.get("/api/h5/read_metadata")
async def read_metadata_endpoint(file_path: str):
    try:
        read_metadata(file_path)
        return {"message": f"Metadata from {file_path} read successfully"}
    except Exception as e:
        logger.error(f"Error reading metadata from {file_path}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/api/h5/print_dataset_details")
async def print_dataset_details_endpoint(file_path: str):
    try:
        print_dataset_details(file_path)
        return {"message": f"Dataset details from {file_path} printed successfully"}
    except Exception as e:
        logger.error(f"Error printing dataset details from {file_path}: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/h5/create_interleaved_vds")
async def create_interleaved_vds_endpoint(request: Request):
    try:
        payload = await request.json()
        base_path = payload.get('base_path')
        num_files = payload.get('num_files')
        output_file = payload.get('output_file')
        
        logger.debug(f"Received request to create interleaved VDS with base_path: {base_path}, num_files: {num_files}, output_file: {output_file}")
        
        if not base_path or not num_files or not output_file:
            raise ValueError("Missing required parameters")
        
        logger.debug("Starting the VDS creation process...")
        create_interleaved_vds(base_path, num_files, output_file)
        logger.debug("VDS creation process completed successfully.")
        return {"message": "Interleaved virtual dataset created successfully"}
    except Exception as e:
        logger.error(f"Error creating interleaved virtual dataset: {e}")
        raise HTTPException(status_code=500, detail=str(e))

def create_interleaved_vds(base_path: str, num_files: int, output_file: str):
    # Example implementation of the VDS creation function
    logger.debug(f"Creating interleaved VDS with base_path: {base_path}, num_files: {num_files}, output_file: {output_file}")
    # Simulate some processing
    try:
        # Insert the logic for creating interleaved VDS here
        logger.debug("Processing files and creating VDS...")
        pass  # Replace with actual implementation
    except Exception as e:
        logger.error(f"Exception in create_interleaved_vds: {e}")
        raise



def start_api(config_file_path, rest_port, secondary_server_address):
    global config_file, secondary_server
    config_file = config_file_path
    secondary_server = secondary_server_address
    try:
        logger.info(f"Starting API with config file: {config_file} on port {rest_port} and with secondary server: {secondary_server} ")
        run(app, host="0.0.0.0", port=rest_port, log_level="warning")
    except Exception as e:
        logger.exception("Error while trying to run the REST api")


def main():
    parser = argparse.ArgumentParser(
        description="Standard DAQ Start Stop REST interface"
    )
    parser.add_argument("config_file", type=str, help="Path to JSON config file.")
    parser.add_argument("--rest_port", type=int, help="Port for REST api", default=5000)
    parser.add_argument(
        "--secondary_server",
        type=str,
        help="Address of the secondary server for synchronization",
    )

    args = parser.parse_args()

    start_api(
        config_file_path=args.config_file,
        rest_port=args.rest_port,
        secondary_server_address=args.secondary_server,
    )


if __name__ == "__main__":
    main()
