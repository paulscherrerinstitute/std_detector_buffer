from conans import ConanFile


class SBD(ConanFile):
    generators = "cmake_find_package"
    requires = [
        "zeromq/4.3.4",
        "gtest/1.13.0",
        "rapidjson/cci.20220822",
        "lz4/1.9.4",
        "date/3.0.1",
        "range-v3/0.12.0",
        "fmt/10.0.0",
        "benchmark/1.6.1",
        "protobuf/3.21.9",
        "argparse/2.9",
        "bsread_receiver/0.13.0@patro_m/psi",
        "redis-plus-plus/1.3.7",
        "hdf5/1.14.1"
    ]

    default_options = {"date:header_only": True,
                       "hdf5:parallel": True,
                       "hdf5:enable_cxx": False}
