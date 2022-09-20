#
# Adds packaging for given TARGET - currently only binaries are supported
#
# Usage:    sdb_package(target_name)
#
# Parameters:
#     target_name : name of target a setting applies to
#
function(sdb_package TARGET)
    include(GNUInstallDirs)
    install(
        TARGETS             ${TARGET}
        EXPORT              ${TARGET}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()
