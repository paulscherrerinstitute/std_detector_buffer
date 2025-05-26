/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "replayer.hpp"

#include <algorithm>
#include <thread>
#include <utility>

#include <zmq.h>

#include "core_buffer/communicator.hpp"
#include "utils/utils.hpp"

using namespace std::chrono_literals;

namespace sbr {
namespace {

constexpr auto zmq_sndhwm = 100;

void* bind_sender_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_PUSH);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));
  if (zmq_bind(socket, stream_address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

} // namespace

replayer::replayer(std::shared_ptr<sbr::state_manager> sm,
                   const utils::DetectorConfig& config,
                   const std::string& stream_address)
    : manager(std::move(sm))
    , zmq_ctx(zmq_ctx_new())
    , stats(config.detector_name, config.stats_collection_period, "none")
{
  static constexpr auto zmq_io_threads = 4;
  zmq_ctx_set(zmq_ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto source_name = fmt::format("{}-image", config.detector_name);
  const std::size_t max_data_bytes = utils::converted_image_n_bytes(config);
  receiver = std::make_unique<cb::Communicator>(
      cb::Communicator{{source_name, max_data_bytes, utils::slots_number(config)},
                       {source_name, zmq_ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}});
  push_socket = bind_sender_socket(zmq_ctx, stream_address);
  auto driver_address =
      fmt::format("{}{}-driver", buffer_config::IPC_URL_BASE, config.detector_name);
  driver_socket = bind_sender_socket(zmq_ctx, driver_address);
}

void replayer::init(std::chrono::seconds logging_period)
{
  auto self = shared_from_this();
  std::thread([self, logging_period]() {
    while (true) {
      self->stats.update_stats(self->manager->get_active_sessions());
      self->stats.print_stats();
      std::this_thread::sleep_for(logging_period);
    }
  }).detach();
}

void replayer::start(const replay_settings& settings)
{
  auto self = shared_from_this();
  std::thread([self, settings]() {
    // todo fill implementation
    self->manager->change_state(reader_state::finished);
  }).detach();
}

} // namespace sbr
