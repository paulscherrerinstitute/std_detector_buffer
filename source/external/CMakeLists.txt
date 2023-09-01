cmake_minimum_required(VERSION 3.17)
project(bitshuffle)

find_package(lz4 REQUIRED)
find_package(HDF5 REQUIRED)
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE _OPENMP USEAVX2)
target_compile_options(${PROJECT_NAME} PRIVATE -mavx2)

target_sources(${PROJECT_NAME}
    PUBLIC
        bitshuffle/bitshuffle.h
        bitshuffle/bitshuffle_core.h
        bitshuffle/bitshuffle_internals.h
        bitshuffle/bshuf_h5filter.h
        bitshuffle/iochain.h
    PRIVATE
        bitshuffle/bitshuffle.c
        bitshuffle/bitshuffle_core.c
        bitshuffle/bshuf_h5filter.c
        bitshuffle/iochain.c
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        lz4::lz4
        hdf5::hdf5
        MPI::MPI_CXX
        OpenMP::OpenMP_CXX
)
