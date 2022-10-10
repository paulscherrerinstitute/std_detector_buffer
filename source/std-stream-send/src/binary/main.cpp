/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <zmq.h>
#include <buffer_utils.hpp>
#include <ram_buffer.hpp>
#include <algorithm>

#include "jungfrau.hpp"
#include "stream_config.hpp"
#include "core_buffer/communicator.hpp"

using namespace std;
using namespace stream_config;
using namespace buffer_config;

void* bind_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_PUB);

  const int sndhwm = STREAM_ZMQ_SNDHWM;
  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_bind(socket, stream_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  return socket;
}

tuple<buffer_utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  if (argc != 3) {
    cout << endl;

    cout << "Usage: std_stream_send_binary [detector_json_filename] [stream_address]" << endl;
    cout << "\tdetector_json_filename: detector config file path." << endl;
    cout << "\tstream_address: address to bind the output stream." << endl;
    cout << endl;

    exit(-1);
  }

  auto config = buffer_utils::read_json_config(string(argv[1]));
  const auto stream_address = string(argv[2]);

  return {config, stream_address};
}

int main(int argc, char* argv[])
{
  auto const [config, stream_address] = read_arguments(argc, argv);

  auto ctx = zmq_ctx_new();
  zmq_ctx_set(ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);
  auto socket = bind_socket(ctx, stream_address);

  // TODO: The module_id here is temporary.
  auto const module_id = 0;
  auto receiver =
      cb::Communicator{{config.detector_name + "-" + std::to_string(module_id) + "-3-converted",
                        BYTES_PER_PACKET - DATA_BYTES_PER_PACKET,
                        config.image_pixel_width * config.image_pixel_height * sizeof(float),
                        buffer_config::RAM_BUFFER_N_SLOTS},
                       {ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}};

  // TODO: This is temporary. * 4 is because of float32 (after conversion).
  const size_t IMAGE_N_BYTES = config.image_pixel_height * config.image_pixel_width * 4;

  ImageMetadata image_meta;
  image_meta.dtype = ImageMetadataDtype::float32;
  image_meta.height = config.image_pixel_height;
  image_meta.width = config.image_pixel_width;

  while (true) {

    auto [image_id, meta_buffer, data_buffer] = receiver.receive();

    image_meta.id = image_id;
    auto frame_meta = reinterpret_cast<JFFrame*>(meta_buffer);

    if (frame_meta->n_missing_packets == 0) {
      image_meta.status = ImageMetadataStatus::good_image;
    }
    else {
      image_meta.status = ImageMetadataStatus::missing_packets;
    }

    zmq_send(socket, &image_meta, sizeof(image_meta), ZMQ_SNDMORE);
    zmq_send(socket, data_buffer, IMAGE_N_BYTES, 0);
  }
}
