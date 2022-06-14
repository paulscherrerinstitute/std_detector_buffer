cmake_minimum_required(VERSION 3.17)

add_library(compiler_warnings INTERFACE)
add_library(${PROJECT_NAME}::compiler_warnings ALIAS compiler_warnings)

target_compile_options(compiler_warnings INTERFACE -Wall -Wextra -Werror)
