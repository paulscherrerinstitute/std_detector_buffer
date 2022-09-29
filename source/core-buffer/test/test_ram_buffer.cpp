/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "ram_buffer.hpp"

#include <span>
#include <gtest/gtest.h>
#include <range/v3/all.hpp>

#include "jungfrau.hpp"

using namespace buffer_config;

TEST(RamBuffer, SimpleStoreSingleFrame)
{
  constexpr size_t DATA_N_BYTES = MODULE_N_PIXELS * 2;
  constexpr size_t SLOTS = 3;
  constexpr uint64_t INITIAL_ID = 12345678;

  RamBuffer buffer("test_detector", sizeof(ModuleFrame), DATA_N_BYTES, SLOTS);

  ModuleFrame frame_meta{};
  frame_meta.id = INITIAL_ID;
  frame_meta.pulse_id = 123523;
  frame_meta.daq_rec = 1234;
  frame_meta.frame_index = 12342300;
  frame_meta.n_recv_packets = 128;
  frame_meta.module_id = 1;

  auto frame =
      ranges::iota_view<uint16_t>(0) | ranges::views::take(MODULE_N_PIXELS) | ranges::to_vector;

  for (auto i : ranges::views::indices(SLOTS)) {
    frame_meta.id = INITIAL_ID + i;
    buffer.write(INITIAL_ID + i, (char*)(&frame_meta), (char*)(frame.data()));
  }

  for (auto i : ranges::views::indices(SLOTS)) {
    auto meta_buffer = (ModuleFrame*)buffer.get_meta(INITIAL_ID + i);
    ASSERT_EQ(meta_buffer->id, INITIAL_ID + i);
    ASSERT_EQ(meta_buffer->pulse_id, frame_meta.pulse_id);
    ASSERT_EQ(meta_buffer->daq_rec, frame_meta.daq_rec);
    ASSERT_EQ(meta_buffer->module_id, frame_meta.module_id);
    ASSERT_EQ(meta_buffer->frame_index, frame_meta.frame_index);
  }

  ranges::span<uint16_t> data_buffer((uint16_t*)buffer.get_data(frame_meta.id), MODULE_N_PIXELS);
  ASSERT_TRUE(ranges::equal(frame, data_buffer));
}
