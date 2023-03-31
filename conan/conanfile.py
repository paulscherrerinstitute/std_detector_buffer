from conans import ConanFile


class SBD(ConanFile):
    generators = "cmake_find_package"
    requires = [
        "zeromq/4.3.4",
        "gtest/1.11.0",
        "rapidjson/cci.20211112",
        "lz4/1.9.3",
        "date/3.0.1",
        "range-v3/0.11.0",
        "fmt/8.1.1",
        "benchmark/1.6.1",
        "protobuf/3.20.0",
        "argparse/2.9",
        "bsread_receiver/0.13.0@patro_m/psi",
        "hdf5/1.13.1"
    ]

    default_options = {"date:header_only": True,
                       "hdf5:parallel": True,
                       "hdf5:enable_cxx": False}
