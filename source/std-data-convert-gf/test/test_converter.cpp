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
} // namespace

TEST(ConverterGf, CheckConversionFrom12bitTo16bitForSouthWest_8x4)
{
  const auto width = 4;
  const auto height = 8;
  const auto module = 1;
  const std::size_t pixels = width * height;
  uint16_t output[pixels] = {};

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SW, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SW, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SW, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SW, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SE, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SE, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SE, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::SE, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NW, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NW, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NW, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NW, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NE, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NE, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // first module
  auto converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NE, module);
  converter->convert(example_data_1, std::span(reinterpret_cast<char*>(output), pixels));
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
  converter = std::make_unique<gf::sdc::Converter>(height, width, gf::quadrant_id::NE, module + 1);
  converter->convert(example_data_2, std::span(reinterpret_cast<char*>(output), pixels));
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

  // Packed bytes from each module.
  const uint16_t SE0[] = {20740, 1552, 4209, 20868, 34328, 6257};
  //  const uint16_t SE1[] = {20804, 17940, 5233, 20932, 50716, 7281};
  //  const uint16_t SW0[] = {4352, 528, 4145, 4480, 33304, 6193};
  //  const uint16_t SW1[] = {4416, 16916, 5169, 4544, 49692, 7217};
  //  const uint16_t NE0[] = {20612, 34312, 2160, 20484, 1536, 112};
  //  const uint16_t NE1[] = {20676, 50700, 3184, 20548, 17924, 1136};
  //  const uint16_t NW0[] = {4224, 33288, 2096, 4096, 512, 48};
  //  const uint16_t NW1[] = {4288, 49676, 3120, 4160, 16900, 1072};

  uint16_t buffer[height * width] = {};
  span<char> output_buffer((char*)(&buffer), sizeof(buffer));
  // 128 bytes = height * width * 2 bytes / pixel
  EXPECT_TRUE(output_buffer.size() == 128);

  gf::sdc::Converter converter(height, width, gf::quadrant_id::SE, 1);
  // 12 bytes = 8 pixels * 1.5 bytes / pixel
  converter.convert(std::span<char>((char*)(&SE0), sizeof(SE0)), output_buffer);
  //  converter.convert(std::span<char>((char*)(&SE1), 12), output_buffer, 0);

//  for (int y = 0; y < height; y++) {
//    for (int x = 0; x < width; x++) {
//      int index = (y * width) + x;
//      std::cout << buffer[index] << ' ';
//    }
//    std::cout << std::endl;
//  }
}
