import requests
import json

BASE_URL = "http://localhost:5000/api"

# Sample user
user = "test_user"

# Config file used in the test
config_file = "/etc/std_daq/configs/gf1.json"

# Test function for the /api/config/get endpoint
def test_get_configuration():
    response = requests.get(f"{BASE_URL}/config/get", params={"user": user})
    print("GET /config/get:", response.status_code, response.json())

# Test function for the /api/config/set endpoint (valid case)
def test_update_configuration_valid():
    # Step 1: Get the current configuration
    get_response = requests.get(f"{BASE_URL}/config/get", params={"user": user})
    
    if get_response.status_code != 200:
        print(f"Failed to get current configuration: {get_response.status_code}, {get_response.json()}")
        return
    
    # Step 2: Modify the image pixel height and width
    new_config = get_response.json()
    new_config["image_pixel_height"] = 2016
    new_config["image_pixel_width"] = 2016

    # Step 3: Send the updated configuration
    response = requests.post(f"{BASE_URL}/config/set", params={"user": user}, json=new_config)
    print("POST /config/set (valid):", response.status_code, response.json())

# Test function for the /api/config/set endpoint (invalid case: change detector name)
def test_update_configuration_invalid():
    new_config = {
        "detector_name": "Changed Detector Name",
        "detector_type": "Test Type",
        "n_modules": 4,
        "bit_depth": 12,
        "image_pixel_height": 2016,
        "image_pixel_width": 2016,
        "start_udp_port": 8000,
        "writer_user_id": 1,
        "submodule_info": {},
        "max_number_of_forwarders_spawned": 2,
        "use_all_forwarders": True,
        "module_sync_queue_size": 10,
        "module_positions": {}
    }
    response = requests.post(f"{BASE_URL}/config/set", params={"user": user}, json=new_config)
    print("POST /config/set (invalid):", response.status_code, response.json())

# Test function for the /api/h5/read_metadata endpoint
def test_read_metadata():
    filename = "/gpfs/test/test-beamline/interleaved_api_virtual_new.h5"
    response = requests.get(f"{BASE_URL}/h5/read_metadata", params={"filename": filename})
    print("GET /h5/read_metadata:", response.status_code, response.json())

# Test function for the /api/h5/get_metadata_status endpoint
def test_get_metadata_status():
    filename = "/gpfs/test/test-beamline/interleaved_api_virtual_new.h5"
    response = requests.get(f"{BASE_URL}/h5/get_metadata_status", params={"filename": filename})
    print("GET /h5/get_metadata_status:", response.status_code, response.json())

# Test function for the /api/h5/create_interleaved_vds endpoint
def test_create_interleaved_vds():
    payload = {
        "base_path": "/gpfs/test/test-beamline/",
        "output_file": "interleaved_api_virtua.h5"
    }
    response = requests.post(f"{BASE_URL}/h5/create_interleaved_vds", json=payload)
    print("POST /h5/create_interleaved_vds:", response.status_code, response.json())

if __name__ == "__main__":
    print("Running tests...")

    test_get_configuration()
    test_update_configuration_valid()
    test_update_configuration_invalid()
    test_read_metadata()
    test_get_metadata_status()
    test_create_interleaved_vds()

    print("Tests completed.")

