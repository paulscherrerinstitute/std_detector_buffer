/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <cmath>

#include <gtest/gtest.h>
#include <range/v3/all.hpp>

using namespace ranges;

namespace {
char example_data_1[] = {0b00100000, 0b00010010, 0b00000010,
                         0b00100000, 0b00110010, 0b00000100}; // 513, 514, 515, 516
char example_data_2[] = {0b01100000, 0b00100110, 0b00000011,
                         0b01100000, 0b01000110, 0b00000101}; // 1538, 1539, 1540, 1541

uint16_t gen_block_1[] = {4096,  512,   48,    20484, 1536,  112,   4160,  16900, 1072, 20548,
                          17924, 1136,  4224,  33288, 2096,  20612, 34312, 2160,  4288, 49676,
                          3120,  20676, 50700, 3184,  4352,  528,   4145,  20740, 1552, 4209,
                          4416,  16916, 5169,  20804, 17940, 5233,  4480,  33304, 6193, 20868,
                          34328, 6257,  4544,  49692, 7217,  20932, 50716, 7281};

} // namespace

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthWest_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::SW);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
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
  EXPECT_TRUE(equal(expected_1, output));
  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
    0,    0,    0, 0,
    0,    0,    0, 0,
    0,    0,    0, 0,
    0,    0,    0, 0,
    513,  514,  0, 0,
    1538, 1539, 0, 0,
    515,  516,  0, 0,
    1540, 1541, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthWest_4x8)
{
  const auto width = 8;
  const auto height = 4;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::SW);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
    0,   0,   0,   0,   0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0,
    513, 514, 515, 516, 0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));

  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
   0,    0,    0,    0,    0, 0, 0, 0,
   0,    0,    0,    0,    0, 0, 0, 0,
   513,  514,  515,  516,  0, 0, 0, 0,
   1538, 1539, 1540, 1541, 0, 0, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthEast_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::SE);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
    0, 0, 0,   0,
    0, 0, 0,   0,
    0, 0, 0,   0,
    0, 0, 0,   0,
    0, 0, 513, 514,
    0, 0, 0,   0,
    0, 0, 515, 516,
    0, 0, 0,   0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));
  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
    0, 0, 0,    0,
    0, 0, 0,    0,
    0, 0, 0,    0,
    0, 0, 0,    0,
    0, 0, 513,  514,
    0, 0, 1538, 1539,
    0, 0, 515,  516,
    0, 0, 1540, 1541
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthEast_4x8)
{
  const auto width = 8;
  const auto height = 4;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::SE);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
    0, 0, 0, 0, 0,   0,   0,   0,
    0, 0, 0, 0, 0,   0,   0,   0,
    0, 0, 0, 0, 513, 514, 515, 516,
    0, 0, 0, 0, 0,   0,   0,   0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));

  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
   0, 0, 0, 0, 0,    0,    0,    0,
   0, 0, 0, 0, 0,    0,    0,    0,
   0, 0, 0, 0, 513,  514,  515,  516,
   0, 0, 0, 0, 1538, 1539, 1540, 1541
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForNorthWest_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::NW);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
    0,   0,   0, 0,
    515, 516, 0, 0,
    0,   0,   0, 0,
    513, 514, 0, 0,
    0,   0,   0, 0,
    0,   0,   0, 0,
    0,   0,   0, 0,
    0,   0,   0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));
  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
   1540, 1541, 0, 0,
   515,  516,  0, 0,
   1538, 1539, 0, 0,
   513,  514,  0, 0,
   0,    0,    0, 0,
   0,    0,    0, 0,
   0,    0,    0, 0,
   0,    0,    0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForNorthWest_4x8)
{
  const auto width = 8;
  const auto height = 4;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::NW);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
    0,   0,   0,   0,   0, 0, 0, 0,
    513, 514, 515, 516, 0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0,
    0,   0,   0,   0,   0, 0, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));

  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
    1538, 1539, 1540, 1541, 0, 0, 0, 0,
    513,  514,  515,  516,  0, 0, 0, 0,
    0,    0,    0,    0,    0, 0, 0, 0,
    0,    0,    0,    0,    0, 0, 0, 0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForNorthEast_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::NE);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
     0, 0, 0,   0,
     0, 0, 515, 516,
     0, 0, 0,   0,
     0, 0, 513, 514,
     0, 0, 0,   0,
     0, 0, 0,   0,
     0, 0, 0,   0,
     0, 0, 0,   0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));
  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
    0, 0, 1540, 1541,
    0, 0, 515,  516,
    0, 0, 1538, 1539,
    0, 0, 513,  514,
    0, 0, 0,    0,
    0, 0, 0,    0,
    0, 0, 0,    0,
    0, 0, 0,    0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForNorthEast_4x8)
{
  const auto width = 8;
  const auto height = 4;
  const auto module = 1;
  const std::size_t pixels = width * height;

  uint16_t output[pixels] = {};
  gf::sdc::Converter converter(height, width, gf::quadrant_id::NE);

  // first module
  converter.convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels), module);
  // clang-format off
  const uint16_t expected_1[] = {
     0, 0, 0, 0, 0,   0,   0,   0,
     0, 0, 0, 0, 513, 514, 515, 516,
     0, 0, 0, 0, 0,   0,   0,   0,
     0, 0, 0, 0, 0,   0,   0,   0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_1, output));

  // second module
  converter.convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels), module + 1);
  // clang-format off
  const uint16_t expected_2[] = {
    0, 0, 0, 0, 1538, 1539, 1540, 1541,
    0, 0, 0, 0, 513,  514,  515,  516,
    0, 0, 0, 0, 0,    0,    0,    0,
    0, 0, 0, 0, 0,    0,    0,    0
  };
  // clang-format on
  EXPECT_TRUE(equal(expected_2, output));
}

TEST(ConverterGf, CheckConversionFromGeneratedBytes)
{
  const auto height = 8;
  const auto width = 8;

  // 96 bytes = height * width * 1.5 bytes / pixel
  span<char> input_buffer((char*)(&gen_block_1), height * width * 1.5);
  EXPECT_TRUE(input_buffer.size() == 96);

  // 128 bytes = height * width * 2 bytes / pixel
  char buffer[height * width * 2] = {};
  span<char> output_buffer((char*)(&buffer), height * width * 2);
  EXPECT_TRUE(output_buffer.size() == 128);

  input_buffer[0] = 0;
  input_buffer[input_buffer.size()-1] = 0;

  output_buffer[0] = 0;
  output_buffer[output_buffer.size()-1] = 0;

  gf::sdc::Converter converter(height, width, gf::quadrant_id::SE);
  converter.convert(input_buffer, output_buffer, 0);
}