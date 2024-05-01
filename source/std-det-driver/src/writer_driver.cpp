/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "writer_driver.hpp"

#include <algorithm>
#include <ranges>
#include <thread>
#include <utility>

#include <zmq.h>

#include "std_buffer/writer_command.pb.h"

using namespace std::chrono_literals;

namespace std_driver {

writer_driver::writer_driver(std::shared_ptr<std_driver::state_manager> sm,
                             const std::string& source_name,
                             utils::DetectorConfig config,
                             std::size_t number_of_writers)
    : manager(std::move(sm))
    , zmq_ctx(zmq_ctx_new())
    , receiver(cb::Communicator{{source_name, 512, buffer_config::RAM_BUFFER_N_SLOTS},
                                {source_name, zmq_ctx, cb::CONN_TYPE_CONNECT, ZMQ_SUB}})
{
  static constexpr auto zmq_io_threads = 4;
  zmq_ctx_set(zmq_ctx, ZMQ_IO_THREADS, zmq_io_threads);

  std::ranges::for_each(std::views::iota(0u, number_of_writers), [&](auto i) {
    sender_sockets.push_back(
        bind_sender_socket(fmt::format("{}-driver-{}", config.detector_name, i)));
    receiver_sockets.push_back(
        connect_to_socket(fmt::format("{}-writer-{}", config.detector_name, i)));
  });
}

void writer_driver::start(const run_settings& settings)
{
  auto self = shared_from_this();
  std::thread([self, settings]() {
    self->send_create_file_requests(settings.path, settings.writer);
    if (self->did_all_writers_acknowledge()) {
      self->manager->change_state(driver_state::waiting_for_first_image);
      self->record_images(settings.n_images);
      if (self->did_all_writers_record_data()) self->send_save_file_requests();
    }
    else
      self->manager->change_state(driver_state::error);
  }).detach();
}

void* writer_driver::bind_sender_socket(const std::string& stream_address)
{
  void* socket = zmq_socket(zmq_ctx, ZMQ_PUSH);

  constexpr auto zmq_sndhwm = 100;
  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const auto address = buffer_config::IPC_URL_BASE + stream_address;
  if (zmq_bind(socket, address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

void* writer_driver::connect_to_socket(const std::string& stream_address)
{
  void* socket = zmq_socket(zmq_ctx, ZMQ_PULL);

  int rcvhwm = 1000;
  if (zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int timeout = 5000;
  if (zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const auto address = buffer_config::IPC_URL_BASE + stream_address;
  if (zmq_connect(socket, address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

void writer_driver::send_create_file_requests(std::string_view base_path, writer_id id)
{
  manager->change_state(driver_state::creating_file);

  std_daq_protocol::WriterAction msg;
  auto createFileCmd = msg.mutable_create_file();
  createFileCmd->set_writer_id(id);

  std::string cmd;

  for (auto i = 0u; i < sender_sockets.size(); ++i) {
    std::string path = fmt::format("{}/file{}.h5", base_path, i);
    createFileCmd->set_path(path);
    msg.SerializeToString(&cmd);
    zmq_send(sender_sockets[i], cmd.c_str(), cmd.size(), 0);
  }
}

bool writer_driver::did_all_writers_acknowledge()
{
  char buffer[512];
  auto files_created = 0u;
  for (auto& socket : receiver_sockets)
    if (auto n_bytes = zmq_recv(socket, buffer, sizeof(buffer), 0); n_bytes > 0) files_created++;

  return files_created == receiver_sockets.size();
}

void writer_driver::record_images(std::size_t n_images)
{
  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  std::string cmd;

  for (auto i = 0u; i < n_images && manager->get_state() == driver_state::recording; i++) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      if (i == 0) manager->change_state(driver_state::recording);
      meta.ParseFromArray(buffer, n_bytes);
      std_daq_protocol::WriterAction action;
      *action.mutable_record_image()->mutable_image_metadata() = meta;
      action.SerializeToString(&cmd);
      zmq_send(sender_sockets[i % sender_sockets.size()], cmd.c_str(), cmd.size(), 0);
    }
  }
}

void writer_driver::send_save_file_requests()
{
  manager->change_state(driver_state::saving_file);

  std_daq_protocol::WriterAction action;
  action.mutable_close_file();
  send_command_to_all_writers(action);

  if (did_all_writers_acknowledge())
    manager->change_state(driver_state::file_saved);
  else
    manager->change_state(driver_state::error);
}

bool writer_driver::did_all_writers_record_data()
{
  std_daq_protocol::WriterAction action;
  action.mutable_confirm_last_image();
  send_command_to_all_writers(action);

  // the timeout is 5 seconds - after that we assume the write failed
  return did_all_writers_acknowledge();
}

void writer_driver::send_command_to_all_writers(const std_daq_protocol::WriterAction& action)
{
  std::string cmd;
  action.SerializeToString(&cmd);

  for (auto& socket : sender_sockets)
    zmq_send(socket, cmd.c_str(), cmd.size(), 0);
}

} // namespace std_driver
