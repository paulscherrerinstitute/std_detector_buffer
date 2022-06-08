cmake_minimum_required(VERSION 3.17)

add_library(settings INTERFACE)
add_library(${PROJECT_NAME}::settings ALIAS settings)

include(sdb_warnings)
target_link_libraries(settings INTERFACE ${PROJECT_NAME}::compiler_warnings)
