cmake_minimum_required(VERSION 3.12)
project(std_detector_buffer LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_OUTPUT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_OUTPUT")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CPACK_PACKAGE_VENDOR "Paul Scherrer Institute")
set(CPACK_GENERATOR "TGZ;RPM")
include(CPack)

include(sdb_settings)
include(sdb_conan)
include(sdb_packaging)

conan_config_install(ITEM ${CMAKE_HOME_DIRECTORY}/conan/settings.yml)
conan_cmake_autodetect(DETECTED_SETTINGS)
conan_cmake_install(
        PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conan
        BUILD missing
        SETTINGS ${DETECTED_SETTINGS}
        SETTINGS compiler.psi=True # ensure that all packages are built or used from local cache
)

include(CTest)
add_subdirectory("source")
