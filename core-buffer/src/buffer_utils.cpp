#include "buffer_utils.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>
#include <zmq.h>

#include "buffer_config.hpp"

using namespace std;
using namespace buffer_config;

void* buffer_utils::connect_socket(void* ctx, const string& buffer_name)
{
  string ipc_address = buffer_config::IPC_URL_BASE + buffer_name;

#ifdef DEBUG_OUTPUT
  cout << "[buffer_utils::connect_socket]";
  cout << " IPC address: " << ipc_address << endl;
#endif

  void* socket = zmq_socket(ctx, ZMQ_SUB);
  if (socket == nullptr) {
    throw runtime_error(zmq_strerror(errno));
  }

  int rcvhwm = BUFFER_ZMQ_RCVHWM;
  if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_connect(socket, ipc_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  return socket;
}

void* buffer_utils::bind_socket(void* ctx, const string& buffer_name)
{
  string ipc_address = IPC_URL_BASE + buffer_name;

#ifdef DEBUG_OUTPUT
  cout << "[buffer_utils::bind_socket]";
  cout << " IPC address: " << ipc_address << endl;
#endif

  void* socket = zmq_socket(ctx, ZMQ_PUB);

  const int sndhwm = BUFFER_ZMQ_SNDHWM;
  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  if (zmq_bind(socket, ipc_address.c_str()) != 0) {
    throw runtime_error(zmq_strerror(errno));
  }

  return socket;
}

buffer_utils::DetectorConfig buffer_utils::read_json_config(const std::string& filename)
{
  std::ifstream ifs(filename);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::Document config_parameters;
  config_parameters.ParseStream(isw);

  return {
      config_parameters["detector_name"].GetString(),
      config_parameters["detector_type"].GetString(),
      config_parameters["n_modules"].GetInt(),
      config_parameters["bit_depth"].GetInt(),
      config_parameters["image_pixel_height"].GetInt(),
      config_parameters["image_pixel_width"].GetInt(),
      static_cast<uint16_t>(config_parameters["start_udp_port"].GetUint())
  };
}
