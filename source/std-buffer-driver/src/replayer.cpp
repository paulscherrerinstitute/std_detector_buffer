/////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "replayer.hpp"

#include <algorithm>
#include <thread>
#include <utility>

#include <zmq.h>

#include "std_buffer/image_buffer.pb.h"
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

void* connect_driver_socket(void* ctx, const std::string& stream_address)
{
  void* socket = zmq_socket(ctx, ZMQ_REQ);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  if (zmq_connect(socket, stream_address.c_str()) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

} // namespace

replayer::replayer(std::shared_ptr<sbr::state_manager> sm,
                   const utils::DetectorConfig& config,
                   const std::string& stream_address)
    : manager(std::move(sm))
    , zmq_ctx(zmq_ctx_new())
    , stats(config.detector_name, config.stats_collection_period, "none")
    , receiver{
          {fmt::format("{}-image", config.detector_name), utils::converted_image_n_bytes(config),
           utils::slots_number(config)},
          {fmt::format("{}-image", config.detector_name), zmq_ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}}

{
  static constexpr auto zmq_io_threads = 4;
  zmq_ctx_set(zmq_ctx, ZMQ_IO_THREADS, zmq_io_threads);

  push_socket = bind_sender_socket(zmq_ctx, stream_address);
  auto driver_address =
      fmt::format("{}{}-driver", buffer_config::IPC_URL_BASE, config.detector_name);
  driver_socket = connect_driver_socket(zmq_ctx, driver_address);
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
  manager->change_state(reader_state::replaying);
  auto self = shared_from_this();
  std::thread([self, settings]() { self->control_reader(settings); }).detach();
  std::thread([self, settings]() { self->forward_images(settings); }).detach();
}

void replayer::control_reader(const replay_settings& settings) const
{
  std_daq_protocol::RequestNextImage request;
  std_daq_protocol::NextImageResponse response;
  std::string cmd;
  char buffer[512];

  request.set_new_request(true);

  for (auto image_id = settings.start_image_id;
       image_id <= settings.end_image_id && manager->get_state() == reader_state::replaying;)
  {
    request.set_image_id(image_id);
    request.SerializeToString(&cmd);

    zmq_send(driver_socket, cmd.c_str(), cmd.size(), 0);
    request.set_new_request(false);

    if (const auto n_bytes = zmq_recv(driver_socket, buffer, sizeof(buffer), 0); n_bytes > 0) {
      response.ParseFromArray(buffer, n_bytes);
      if (response.has_ack())
        image_id = response.ack().image_id() + 1;
      else
        break;
      {
        const uint64_t wait_for_id = response.ack().image_id();
        std::unique_lock lock(mutex);
        cv.wait(lock, [this, wait_for_id]() {
          return last_sent_id.load(std::memory_order_acquire) >= wait_for_id;
        });
      }
    }
  }
  manager->change_state(reader_state::finishing);
}

void replayer::forward_images(const replay_settings& settings)
{
  char buffer[512];
  std_daq_protocol::ImageMetadata meta;

  for (auto n_images = 0ul; manager->get_state() == reader_state::replaying;) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      meta.ParseFromArray(buffer, n_bytes);

      if (meta.image_id() <= settings.end_image_id) {
        auto data_header = utils::stream::prepare_array10_header(meta);
        auto encoded_c = data_header.c_str();
        auto data = receiver.get_data(meta.image_id());

        zmq_send(push_socket, encoded_c, data_header.length(), ZMQ_SNDMORE);
        zmq_send(push_socket, data, meta.size(), 0);

        manager->update_image_count(++n_images);
        last_sent_id.store(meta.image_id(), std::memory_order_release);
      }
      cv.notify_one();
    }
  }
  manager->change_state(reader_state::finished);
}
} // namespace sbr
