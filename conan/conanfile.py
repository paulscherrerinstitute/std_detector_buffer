from conans import ConanFile


class SBD(ConanFile):
    generators = "cmake_find_package"
    requires = [
        "zeromq/4.3.5",
        "gtest/1.14.0",
        "nlohmann_json/3.11.3",
        "lz4/1.9.4",
        "range-v3/0.12.0",
        "fmt/10.2.1",
        "benchmark/1.8.3",
        "protobuf/3.21.12",
        "argparse/3.0",
        "bsread_receiver/0.16.1@patro_m/psi",
        "redis-plus-plus/1.3.12",
        "c-blosc2/2.13.1",
        "spdlog/1.13.0",
        "hdf5/1.14.3",
        "hash-library/8.0",
        "boost/1.84.0"
    ]

    default_options = {"date:header_only": True,
                       "hdf5:parallel": False,
                       "hdf5:enable_cxx": False,
                       "c-blosc2:with_zstd": False,
                       "spdlog:header_only": True}
