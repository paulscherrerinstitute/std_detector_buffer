find_package(Protobuf REQUIRED)

add_library(protocol)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS image_metadata.proto)
target_link_libraries(protocol protobuf::libprotobuf)
target_sources(protocol
        PRIVATE
            ${PROTO_SRCS}
            ${PROTO_HDRS})
