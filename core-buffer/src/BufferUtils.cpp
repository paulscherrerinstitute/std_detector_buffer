#include "BufferUtils.hpp"

#include <sstream>
#include <buffer_config.hpp>
#include <zmq.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/document.h>

using namespace std;
using namespace buffer_config;

void* BufferUtils::connect_socket(void* ctx, const string& detector_name, const string& stream_name)
{
    string ipc_address = buffer_config::IPC_URL_BASE + detector_name + "-" + stream_name;
    
#ifdef DEBUG_OUTPUT
    cout << "[BufferUtils::connect_socket]";
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

void* BufferUtils::bind_socket(void* ctx, const string& detector_name, const string& stream_name)
{
    string ipc_address = IPC_URL_BASE + detector_name + "-" + stream_name;

#ifdef DEBUG_OUTPUT
    cout << "[BufferUtils::bind_socket]";
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

BufferUtils::DetectorConfig BufferUtils::read_json_config(
        const std::string& filename)
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
            config_parameters["image_height"].GetInt(),
            config_parameters["image_width"].GetInt(),
            config_parameters["start_udp_port"].GetInt(),
    };
}
