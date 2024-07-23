import zmq
import logging
from pathlib import Path
from stats_logger import StatsLogger
import os
import h5py
import numpy as np


class EventFilter(logging.Filter):
    def filter(self, record):
        record.event = "[event]"
        return True

ctx = zmq.Context()
stats_logger = StatsLogger(ctx)



logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(event)s %(levelname)s: %(message)s",
    handlers=[logging.StreamHandler()],
)
logger = logging.getLogger(__name__)
logger.addFilter(EventFilter())


def read_metadata(file_path: str) -> dict:
    """Reads and returns the first 15 metadata entries from an HDF5 file as a dictionary."""
    metadata_dict = {}

    # Open the HDF5 file
    with h5py.File(file_path, "r") as file:
        # Access the metadata dataset
        metadata = file["interleaved_metadata"]

        # Read the entire dataset into a numpy array
        metadata_array = np.array(metadata, dtype=metadata.dtype)

        # Convert metadata to a dictionary and limit to first 15 entries
        metadata_list = []
        for i, entry in enumerate(metadata_array):
            metadata_list.append({
                "image_id": entry["image_id"].item(),
                "status": entry["status"].item()
            })
        
        metadata_dict["metadata"] = metadata_list

    return metadata_dict

def numpy_to_native(obj):
    """Convert numpy objects to native Python types."""
    if isinstance(obj, np.ndarray):
        return obj.tolist()
    if isinstance(obj, np.generic):
        return obj.item()
    if isinstance(obj, dict):
        return {key: numpy_to_native(value) for key, value in obj.items()}
    if isinstance(obj, (list, tuple)):
        return type(obj)(numpy_to_native(item) for item in obj)
    return obj


async def get_dataset_details(file_path: str) -> dict:
    """ Gather and return details of datasets including metadata and size from an HDF5 file. """
    details = {}

    with h5py.File(file_path, 'r') as file:
        details["file"] = file_path
        details["datasets"] = []

        def gather_info(name, node):
            if isinstance(node, h5py.Dataset):
                dataset_info = {
                    "name": name,
                    "shape": numpy_to_native(node.shape),
                    "dtype": str(node.dtype),
                    "size": numpy_to_native(node.size),
                    "metadata": {key: numpy_to_native(node.attrs[key]) for key in node.attrs.keys()} if node.attrs else {}
                }
                details["datasets"].append(dataset_info)

        file.visititems(gather_info)

    return details

async def print_dataset_details(file_path):
    """Print details of datasets including metadata and size from an HDF5 file."""
    # Open the HDF5 file
    with h5py.File(file_path, "r") as file:
        print(f"Opening file: {file_path}")

        def print_info(name, node):
            if isinstance(node, h5py.Dataset):
                print(f"\nDataset Name: {name}")
                print(f" - Shape: {node.shape}")
                print(f" - Dtype: {node.dtype}")
                print(f" - Size: {node.size} elements")
                # Print dataset metadata
                if node.attrs:
                    for key, value in node.attrs.items():
                        print(f" - Metadata {key}: {value}")
                else:
                    print(" - No metadata for this dataset.")

        file.visititems(print_info)  # Visit each item in the HDF5 file

def create_interleaved_vds(base_path, num_files, output_file):
    """Create interleaved virtual datasets for both data and metadata from multiple HDF5 files."""
    files = [os.path.join(base_path, f) for f in os.listdir(base_path) if f.endswith('.h5') and os.path.isfile(os.path.join(base_path, f))]


    # Determine the shape and dtype by inspecting the first file
    with h5py.File(files[0], 'r') as sample_file:
        data_shape = sample_file['gf-teststand/data'].shape
        data_dtype = sample_file['gf-teststand/data'].dtype
        metadata_shape = sample_file['gf-teststand/metadata'].shape
        metadata_dtype = sample_file['gf-teststand/metadata'].dtype
        logger.info(f"Data found: {data_shape} and {data_dtype}. Metadata found: {metadata_shape} and {metadata_dtype}.")

    # Setup the virtual layouts for data and metadata
    total_images = data_shape[0] * num_files
    vlayout_data = h5py.VirtualLayout(shape=(total_images, data_shape[1], data_shape[2]), dtype=data_dtype)
    vlayout_metadata = h5py.VirtualLayout(shape=(total_images,), dtype=metadata_dtype)

    # Interleave data from each file
    for i, filename in enumerate(files):
        logger.info(f"Linking file {filename} (number {i}) into the virtual dataset...")
        vsource_data = h5py.VirtualSource(filename, 'gf-teststand/data', shape=data_shape)
        vsource_metadata = h5py.VirtualSource(filename, 'gf-teststand/metadata', shape=metadata_shape)

        for j in range(data_shape[0]):
            layout_index = j * num_files + i
            vlayout_data[layout_index] = vsource_data[j]
            vlayout_metadata[layout_index] = vsource_metadata[j]

    # Create the output file and add the virtual datasets
    with h5py.File(os.path.join(base_path, output_file), 'w', libver='latest') as vds_file:
        metadata_dtype = np.dtype([('image_id', 'uint64'), ('status', 'uint64')])
        fill_value_metadata = np.array([(0, 0)], dtype=metadata_dtype)[0]

        vds_file.create_virtual_dataset('interleaved_data', vlayout_data, fillvalue=0)
        vds_file.create_virtual_dataset('interleaved_metadata', vlayout_metadata, fillvalue=fill_value_metadata)
        logger.info("Interleaved virtual datasets created.")
