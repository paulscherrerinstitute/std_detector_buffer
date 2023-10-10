cmake_minimum_required(VERSION 3.17)
project(utils)

find_package(argparse REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        include/utils/args.hpp
        include/utils/image_size_calc.hpp
        include/utils/detector_config.hpp
        include/utils/get_metadata_dtype.hpp
        include/utils/utils.hpp
    PRIVATE
        src/image_size_calc.cpp
        src/detector_config.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE include/utils)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        argparse::argparse
        fmt::fmt
        utils::stats
        utils::log
        utils::version
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
    PRIVATE
        nlohmann_json::nlohmann_json
        detectors::detectors
)

add_subdirectory(log)
add_subdirectory(stats)
add_subdirectory(version)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
