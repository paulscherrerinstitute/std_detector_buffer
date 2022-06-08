find_package(MPI REQUIRED)
find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

find_package(HDF5 REQUIRED)
add_library(std-det-writer-lib)
target_include_directories(std-det-writer-lib
        PUBLIC include
)

target_link_libraries(std-det-writer-lib
    PRIVATE
        bitshuffle::bitshuffle
        MPI::MPI_CXX
        core-buffer-lib
)

target_sources(std-det-writer-lib
    PRIVATE
        src/JFH5Writer.cpp
        src/WriterStats.cpp
)

add_executable(std_det_writer src/main.cpp)

set_target_properties(std_det_writer PROPERTIES OUTPUT_NAME std_det_writer)

target_link_libraries(std_det_writer
    PRIVATE
        std-det-writer-lib
        core-buffer-lib
        date::date
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        hdf5::hdf5
        MPI::MPI_CXX
)
