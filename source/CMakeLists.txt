cmake_minimum_required(VERSION 3.12)

add_subdirectory(external)

add_subdirectory("core-buffer")
add_subdirectory("detectors")
add_subdirectory("std-data-assembler")
add_subdirectory("std-data-convert")
add_subdirectory("std-det-writer")
add_subdirectory("std-stream-send")
add_subdirectory("std-udp-recv")
add_subdirectory("std-udp-sync")
add_subdirectory("std-udp-tools")
