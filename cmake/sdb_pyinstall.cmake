cmake_minimum_required(VERSION 3.17)

find_program(PY_INSTALLER "pyinstaller" REQUIRED)

function(add_pyinstall_target TARGET)
    cmake_parse_arguments(PYINSTALL_SETTINGS "" "SOURCE" "" ${ARGN})

    if(NOT PYINSTALL_SETTINGS_SOURCE)
        message(FATAL_ERROR "Source needs to be specified for pyinstall target ${TARGET}!")
    endif()

    add_custom_target(${TARGET} ALL
        COMMAND ${PY_INSTALLER} --distpath ${CMAKE_CURRENT_BINARY_DIR} --onefile ${PYINSTALL_SETTINGS_SOURCE}
        COMMENT "Building ${TARGET}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    include(GNUInstallDirs)
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
        TYPE BIN
    )
endfunction()
