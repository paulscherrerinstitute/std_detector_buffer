find_package(Protobuf REQUIRED)

add_library(protocol)

target_include_directories(protocol
        PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
            ${protobuf_INCLUDE_DIRS}
        )

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS image_metadata.proto)
target_link_libraries(protocol ${protobuf_LIBRARIES})
target_sources(protocol
        PRIVATE
            ${PROTO_SRCS}
            ${PROTO_HDRS})

#protobuf_generate_python(PROTO_PY image_metadata.proto)
#add_custom_target(protocol_py ALL DEPENDS ${PROTO_PY})
#file(COPY ${PROTO_PY} DESTINATION std_buffer/)