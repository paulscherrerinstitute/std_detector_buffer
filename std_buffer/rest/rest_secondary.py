import argparse
import hashlib
import json
import logging

import zmq
from fastapi import FastAPI, HTTPException, Request, Query
from fastapi.middleware.cors import CORSMiddleware
from stats_logger import StatsLogger
from utils import EventFilter
from uvicorn import run

logging.basicConfig(
    level=logging.INFO,
    format="[event] %(asctime)s %(levelname)s: %(message)s",
    handlers=[logging.StreamHandler()],
)
logger = logging.getLogger(__name__)
logger.addFilter(EventFilter())

# FastAPI app initialization
app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

ctx = zmq.Context()
stats_logger = StatsLogger(ctx)

# defining global 
config_file = None


def get_config_file():
    return config_file

@app.post("/api/config/set")
async def update_configuration(request: Request, user: str = Query(...)):
    global config_file
    new_config = await request.json()
    logger.info(f"User {user} received new configuration: {new_config}")
    try:
        # Overwrite the local file
        logger.info(f"Updating {config_file}...")
        with open(config_file, "w") as file:
            json.dump(new_config, file, indent=4)
        logger.info(f"Successfully updated configuration file: {config_file}")
        stats_logger.log_config_change("set", "primary_server", True)
        return {"message": "Configuration updated successfully"}
    except Exception as e:
        logger.info(f"Error writing to configuration file: {e}")
        stats_logger.log_config_change("set", "primary_server", False)
        raise HTTPException(status_code=500, detail=str(e))

def start_secondary_api(config_file_path, rest_port):
    global config_file
    config_file = config_file_path
    try:
        logger.info(f"Starting secondary API with config file: {config_file} on port {rest_port}")
        run(app, host="0.0.0.0", port=rest_port, log_level="info")
    except Exception as e:
        logger.exception("Error while trying to run the REST api")

def main():
    parser = argparse.ArgumentParser(description="Secondary Config Update REST interface")
    parser.add_argument("config_file", type=str, help="Path to JSON config file.")
    parser.add_argument("--rest_port", type=int, help="Port for REST api", default=5001)

    args = parser.parse_args()

    start_secondary_api(config_file_path=args.config_file, rest_port=args.rest_port)

if __name__ == "__main__":
    main()
