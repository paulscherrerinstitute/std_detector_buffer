cmake_minimum_required(VERSION 3.17)

find_program(PY_INSTALLER "pyinstaller" REQUIRED)

function(add_pyinstall_target TARGET)
    cmake_parse_arguments(PYINSTALL_SETTINGS "" "SOURCE" "HIDDEN_IMPORTS" ${ARGN})
    if(NOT TARGET pyinstall)
        add_custom_target(pyinstall)
        set_target_properties(pyinstall
            PROPERTIES
                EXCLUDE_FROM_ALL TRUE
                EXCLUDE_FROM_DEFAULT_BUILD TRUE
        )
    endif()
    add_dependencies(pyinstall ${TARGET})

    if(NOT PYINSTALL_SETTINGS_SOURCE)
        message(FATAL_ERROR "Source needs to be specified for pyinstall target ${TARGET}!")
    endif()
    set(_HIDDEN_OPTIONS "")
    if(PYINSTALL_SETTINGS_HIDDEN_IMPORTS)
        foreach(arg ${PYINSTALL_SETTINGS_HIDDEN_IMPORTS})
            set(_HIDDEN_OPTIONS "${_HIDDEN_OPTIONS} --hidden-import  ${arg}")
        endforeach()
    endif()

    add_custom_target(${TARGET}
        COMMAND ${PY_INSTALLER} --distpath ${CMAKE_CURRENT_BINARY_DIR} --onefile ${PYINSTALL_SETTINGS_SOURCE}
            ${_HIDDEN_OPTIONS}
        COMMENT "Building ${TARGET}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    include(GNUInstallDirs)
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
        TYPE BIN
        OPTIONAL
    )
endfunction()
