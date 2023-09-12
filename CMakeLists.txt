cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(sdb_version)

project(std_detector_buffer
    LANGUAGES CXX
    VERSION ${SDB_VERSION}
    HOMEPAGE_URL "https://git.psi.ch/controls-ci/std_detector_buffer"
)

include(sdb_settings)
set(CMAKE_CXX_STANDARD 20)

include(sdb_conan)
conan_config_install(ITEM ${CMAKE_HOME_DIRECTORY}/conan/settings.yml)
conan_config_install(ITEM ${CMAKE_HOME_DIRECTORY}/conan/remotes.json)
conan_cmake_autodetect(DETECTED_SETTINGS)
conan_cmake_install(
    PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conan
    BUILD missing
    SETTINGS ${DETECTED_SETTINGS}
    SETTINGS compiler.psi=True # ensure that all packages are built or used from local cache
    SETTINGS os=Linux
    SETTINGS os_build=Linux
)

include(sdb_packaging)
set(CPACK_PACKAGE_VENDOR "Paul Scherrer Institute")
set(CPACK_GENERATOR "RPM")
include(CPack)

include(CTest)
add_subdirectory("source")


include(sdb_pyinstall)
add_subdirectory("pyinstaller")
