from pathlib import Path

import h5py
import numpy as np


def read_metadata(file_path):
    """Reads and prints the metadata from an HDF5 file."""
    # Open the HDF5 file
    with h5py.File(file_path, "r") as file:
        # Access the metadata dataset
        metadata = file["interleaved_metadata"]

        # Read the entire dataset into a numpy array
        metadata_array = np.array(metadata, dtype=metadata.dtype)

        # Print the metadata
        print("Metadata Contents:")
        for entry in metadata_array:
            print(f"Image ID: {entry['image_id']}, Status: {entry['status']}")


def print_dataset_details(file_path):
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
    files = [f"{base_path}/file{i}.h5" for i in range(num_files)]

    # Determine the shape and dtype by inspecting the first file
    with h5py.File(files[0], "r") as sample_file:
        data_shape = sample_file["gf-teststand/data"].shape
        data_dtype = sample_file["gf-teststand/data"].dtype
        metadata_shape = sample_file["gf-teststand/metadata"].shape
        metadata_dtype = sample_file["gf-teststand/metadata"].dtype

    # Setup the virtual layouts for data and metadata
    total_images = data_shape[0] * num_files
    vlayout_data = h5py.VirtualLayout(
        shape=(total_images, data_shape[1], data_shape[2]), dtype=data_dtype
    )
    vlayout_metadata = h5py.VirtualLayout(shape=(total_images,), dtype=metadata_dtype)

    # Interleave data from each file
    for i, filename in enumerate(files):
        vsource_data = h5py.VirtualSource(
            filename, "gf-teststand/data", shape=data_shape
        )
        vsource_metadata = h5py.VirtualSource(
            filename, "gf-teststand/metadata", shape=metadata_shape
        )

        for j in range(data_shape[0]):
            layout_index = j * num_files + i
            vlayout_data[layout_index] = vsource_data[j]
            vlayout_metadata[layout_index] = vsource_metadata[j]

    # Create the output file and add the virtual datasets
    print(output_file)
    with h5py.File(output_file, "w", libver="latest") as vds_file:
        metadata_dtype = np.dtype([("image_id", "uint64"), ("status", "uint64")])
        fill_value_metadata = np.array([(0, 0)], dtype=metadata_dtype)[0]

        vds_file.create_virtual_dataset("interleaved_data", vlayout_data, fillvalue=0)
        vds_file.create_virtual_dataset(
            "interleaved_metadata", vlayout_metadata, fillvalue=fill_value_metadata
        )
        print("Interleaved virtual datasets created.")
