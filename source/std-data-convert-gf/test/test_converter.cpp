/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <cmath>

#include <gtest/gtest.h>
#include <range/v3/all.hpp>

using namespace ranges;

namespace {
char array[] = {0b00100000, 0b00010010, 0b00000010,
                0b00100000, 0b00110010, 0b00000100}; // 513, 514, 515, 516
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthWest_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width);

  converter.convert(array, std::span(reinterpret_cast<char*>(output), pixels));
  // clang-format off
  const uint16_t expected[] = {
    0,   0,   0, 0,
    0,   0,   0, 0,
    0,   0,   0, 0,
    0,   0,   0, 0,
    513, 514, 0, 0,
    0,   0,   0, 0,
    515, 516, 0, 0,
    0,   0,   0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthWest_4x8)
{
  const auto width = 8;
  const auto height = 4;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width);

  converter.convert(array, std::span(reinterpret_cast<char*>(output), pixels));
  // clang-format off
  const uint16_t expected[] = {
    0,   0,   0,   0,   0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0,
    513, 514, 515, 516, 0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected, output));
}
