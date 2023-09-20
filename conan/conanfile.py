from conans import ConanFile


class SBD(ConanFile):
    generators = "cmake_find_package"
    requires = [
        "zeromq/4.3.4",
        "gtest/1.13.0",
        "nlohmann_json/3.11.2",
        "lz4/1.9.4",
        "range-v3/0.12.0",
        "fmt/10.0.0",
        "benchmark/1.6.1",
        "protobuf/3.21.12",
        "argparse/2.9",
        "bsread_receiver/0.14.0@patro_m/psi",
        "redis-plus-plus/1.3.7",
        "c-blosc2/2.10.0",
        "spdlog/1.12.0",
        "hdf5/1.14.1",
        "zlib/1.2.13"
    ]

    default_options = {"date:header_only": True,
                       "hdf5:parallel": False,
                       "hdf5:enable_cxx": False,
                       "c-blosc2:with_zstd": False,
                       "spdlog:header_only": True}
