/////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "writer_driver.hpp"

#include <algorithm>
#include <ranges>
#include <thread>
#include <chrono>
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
  });
}

void writer_driver::start()
{
  auto self = shared_from_this();
  std::thread([self]() {
    self->send_create_file_requests();
    if (self->are_all_files_created()) {
      self->manager->change_state(driver_state::waiting_for_first_image);
      self->record_images();
      self->send_save_file_requests();
      if (self->are_all_files_saved()) self->manager->change_state(driver_state::file_saved);
    }
  }).detach();
}

void* writer_driver::bind_sender_socket(const std::string& stream_address)
{
  constexpr auto zmq_sndhwm = 100;

  void* socket = zmq_socket(zmq_ctx, ZMQ_PUSH);

  if (zmq_setsockopt(socket, ZMQ_SNDHWM, &zmq_sndhwm, sizeof(zmq_sndhwm)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const int linger = 0;
  if (zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger)) != 0)
    throw std::runtime_error(zmq_strerror(errno));

  const auto address = buffer_config::IPC_URL_BASE + stream_address;
  if (zmq_bind(socket, address.c_str()) != 0) throw std::runtime_error(zmq_strerror(errno));

  return socket;
}

void writer_driver::send_create_file_requests()
{
  manager->change_state(driver_state::creating_file);

  std_daq_protocol::WriterAction msg;
  auto createFileCmd = msg.mutable_create_file();
  // TODO: writer id provided by user
  createFileCmd->set_writer_id(0);

  std::string cmd;

  for (auto i = 0u; i < sender_sockets.size(); ++i) {
    // TODO: directory handling
    std::string path = fmt::format("/gpfs/test/test-beamline/file{}.h5", i);
    createFileCmd->set_path(path);
    msg.SerializeToString(&cmd);
    zmq_send(sender_sockets[i], cmd.c_str(), cmd.size(), 0);
  }
}

bool writer_driver::are_all_files_created()
{
  // TODO: proper implementation when listening part will be implemented
  std::this_thread::sleep_for(300ms);
  return true;
}

void writer_driver::record_images()
{
  char buffer[512];
  std_daq_protocol::ImageMetadata meta;
  std::string cmd;

  auto i = 0u;
  for (auto ii = 0; ii < 160000;) {
    if (auto n_bytes = receiver.receive_meta(buffer); n_bytes > 0) {
      if (ii == 0) manager->change_state(driver_state::recording);
      ii++;
      meta.ParseFromArray(buffer, n_bytes);
      std_daq_protocol::WriterAction action;
      *action.mutable_record_image()->mutable_image_metadata() = meta;
      action.SerializeToString(&cmd);
      zmq_send(sender_sockets[i], cmd.c_str(), cmd.size(), 0);
      i = (i + 1) % sender_sockets.size();
    }
  }
}

void writer_driver::send_save_file_requests()
{
  std::string cmd;
  std::this_thread::sleep_for(std::chrono::seconds(10));
  std_daq_protocol::WriterAction action;
  action.mutable_close_file();
  action.SerializeToString(&cmd);
  for (auto ii = 0u; ii < sender_sockets.size(); ++ii)
    zmq_send(sender_sockets[ii], cmd.c_str(), cmd.size(), 0);
}

bool writer_driver::are_all_files_saved()
{
  // TODO: proper implementation when listening part will be implemented
  std::this_thread::sleep_for(300ms);
  return true;
}

} // namespace std_driver
