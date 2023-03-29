/////////////////////////////////////////////////////////////////////
// Copyright (c) 2023 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include <bsread_receiver/receiver.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

int main(int, char**)
{
  bsrec::receiver receiver("tcp://localhost:5555");

  while (true) {
    auto msg = receiver.receive();
    fmt::print(">>>> Message received: pulseId: {} channels: {}\n", msg.pulse_id,
               msg.channels->size());
    for (const auto& ch : *msg.channels)
      fmt::print(">> CH: {}, type:{}, size:{}, shape:[{}]", ch.name, ch.type, ch.buffer_size,
                 fmt::join(ch.shape, ", "));
  }
  return 0;
}
