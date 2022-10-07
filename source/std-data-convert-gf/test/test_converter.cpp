/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <gtest/gtest.h>

TEST(ConverterGf, CheckConversionFrom12bitTo16bit)
{
  const std::size_t pixels = 4;
  char array[] = {0b00100000, 0b00010010, 0b00000010,
                  0b00100000, 0b00110010, 0b00000100}; // 513, 514, 515, 516
  uint16_t output[pixels];
  gf::sdc::Converter converter(pixels);

  converter.convert(array, std::span(reinterpret_cast<char*>(output), pixels * 2));

  EXPECT_EQ(513, output[0]);
  EXPECT_EQ(514, output[1]);
  EXPECT_EQ(515, output[2]);
  EXPECT_EQ(516, output[3]);
}
