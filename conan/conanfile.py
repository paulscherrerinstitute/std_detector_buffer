from conans import ConanFile


class SBD(ConanFile):
    generators = "cmake_find_package"
    requires = [
        "zeromq/4.3.4",
        "gtest/1.11.0",
        "rapidjson/cci.20211112",
        "lz4/1.9.3",
        "date/3.0.1",
    ]
    default_options = {"date:header_only": True}
