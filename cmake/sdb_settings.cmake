cmake_minimum_required(VERSION 3.25)

set(HDF5_PREFER_PARALLEL ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

include(sdb_add_uts)

add_library(settings INTERFACE)
add_library(${PROJECT_NAME}::settings ALIAS settings)

include(sdb_warnings)
target_link_libraries(settings INTERFACE ${PROJECT_NAME}::compiler_warnings)
