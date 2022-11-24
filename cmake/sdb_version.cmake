cmake_minimum_required(VERSION 3.12)

if(DEFINED $ENV{PACKAGE_VERSION})
    set(SDB_VERSION $ENV{PACKAGE_VERSION})
else()
    set(SDB_VERSION 0.2.3)
endif()
