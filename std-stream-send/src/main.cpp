#include <iostream>
#include <zmq.h>
#include <buffer_utils.hpp>
#include <ram_buffer.hpp>

#include "stream_config.hpp"
#include <algorithm>

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

    cout << "Usage: std_stream_send_binary [detector_json_filename] [stream_address]"
         << endl;
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

  auto receiver = buffer_utils::connect_socket(ctx, config.detector_name + "-assembler");

  const size_t IMAGE_N_BYTES = config.image_pixel_height * config.image_pixel_width * config.bit_depth / 8;

  RamBuffer image_buffer(config.detector_name + "_assembler", sizeof(ImageMetadata), IMAGE_N_BYTES,
                         RAM_BUFFER_N_SLOTS);

  ImageMetadata meta;

  while (true) {

    // Receive the image id to forward.
    zmq_recv(receiver, &meta, sizeof(meta), 0);
    // gets the image data
    char* dst_data = image_buffer.get_data(meta.id);

    zmq_send(socket, &meta, sizeof(meta), ZMQ_SNDMORE);
    zmq_send(socket, dst_data, IMAGE_N_BYTES, 0);
  }
}
