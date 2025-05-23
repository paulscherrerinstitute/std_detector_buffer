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
std::size_t get_uncompressed_size(const std_daq_protocol::ImageMetadata& data)
{
  return data.width() * data.height() * utils::get_bytes_from_metadata_dtype(data.dtype());
}

} // namespace

replayer::replayer(std::shared_ptr<sbr::state_manager> sm,
                   const utils::DetectorConfig& config,
                   const std::string& root_dir,
                   const std::string& db_address)
    : manager(std::move(sm))
    , zmq_ctx(zmq_ctx_new())
    , redis_handler(config.detector_name, db_address, 1)
    , reader(root_dir + config.detector_name, config.bit_depth / 8)
    , stats(config.detector_name, config.stats_collection_period, "none")
{
  static constexpr auto zmq_io_threads = 4;
  zmq_ctx_set(zmq_ctx, ZMQ_IO_THREADS, zmq_io_threads);

  const auto sync_name = fmt::format("{}-image", config.detector_name);
  const std::size_t max_data_bytes = utils::converted_image_n_bytes(config);
  sender = std::make_unique<cb::Communicator>(
      cb::Communicator{{sync_name, max_data_bytes, utils::slots_number(config)},
                       {sync_name, zmq_ctx, cb::CONN_TYPE_BIND, ZMQ_PUB}});
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
    auto zmq_flags = 0;
    self->redis_handler.prepare_receiving(settings.start_image_id, settings.end_image_id);
    self->manager->change_state(reader_state::replaying);
    for (auto sent_images = 0ul; auto data = self->redis_handler.receive(); sent_images++) {
      if (self->manager->get_state() != reader_state::replaying) break;
      const auto image_id = data->metadata().image_id();
      const auto uncompressed_size = get_uncompressed_size(data->metadata());
      self->reader.read(image_id, {self->sender->get_data(image_id), uncompressed_size},
                        data->offset(), data->metadata().size());

      data->mutable_metadata()->set_size(uncompressed_size);
      data->mutable_metadata()->set_compression(std_daq_protocol::none);

      std::string meta_buffer_send;
      data->metadata().SerializeToString(&meta_buffer_send);
      self->sender->send(image_id, meta_buffer_send, nullptr, zmq_flags);
      std::this_thread::sleep_for(std::chrono::milliseconds(10)); // todo this has to be controlled
      zmq_flags = ZMQ_NOBLOCK;                                               // the first transmission is blocking
      self->manager->update_image_count(sent_images);
    }
    self->manager->change_state(reader_state::finished);
  }).detach();
}

} // namespace sbr
