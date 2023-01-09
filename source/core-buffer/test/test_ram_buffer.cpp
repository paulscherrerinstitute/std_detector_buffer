/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "core_buffer/ram_buffer.hpp"

#include <span>
#include <gtest/gtest.h>
#include <range/v3/all.hpp>

#include "detectors/jungfrau.hpp"

using namespace buffer_config;

TEST(RamBuffer, SimpleStoreSingleFrame)
{
  constexpr size_t DATA_N_BYTES = MODULE_N_PIXELS * 2;
  constexpr size_t SLOTS = 3;
  constexpr uint64_t INITIAL_ID = 12345678;

  RamBuffer buffer("test_detector", DATA_N_BYTES, SLOTS);

  auto frame =
      ranges::iota_view<uint16_t>(0) | ranges::views::take(MODULE_N_PIXELS) | ranges::to_vector;

  for (auto i : ranges::views::indices(SLOTS)) {
    buffer.write(INITIAL_ID + i, (char*)(frame.data()));

    ranges::span<uint16_t> data_buffer((uint16_t*)buffer.get_data(INITIAL_ID + i), MODULE_N_PIXELS);
    ASSERT_TRUE(ranges::equal(frame, data_buffer));
  }
}
