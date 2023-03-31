/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <bsread_receiver/receiver.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

#include "utils/args.hpp"
#include "utils/detector_config.hpp"

namespace {

std::tuple<utils::DetectorConfig, std::string> read_arguments(int argc, char* argv[])
{
  auto program = utils::create_parser("std_bsread_recv");
  program.add_argument("stream_address").help("address to bind input stream");
  program = utils::parse_arguments(program, argc, argv);
  return {utils::read_config_from_json_file(program.get("detector_json_filename")),
          program.get("stream_address")};
}
} // namespace

int main(int argc, char* argv[])
{
  const auto [config, stream_address] = read_arguments(argc, argv);
  bsrec::receiver receiver(stream_address, bsrec::socket_type::pull);

  while (true) {
    auto msg = receiver.receive();
    if (msg.channels != nullptr) {
      fmt::print(">>>> Message received: pulseId: {} channels: {}\n", msg.pulse_id,
                 msg.channels->size());
      for (const auto& ch : *msg.channels)
        fmt::print(">> CH: {}, type:{}, size:{}, shape:[{}]", ch.name, ch.type, ch.buffer_size,
                   fmt::join(ch.shape, ", "));
      fmt::print("\n");
      std::fflush(stdout);
    }
  }
  return 0;
}
