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
  gf::sdc::Converter converter(pixels);

  auto converted = converter.convert(array);

  EXPECT_EQ(4, converted.size());
  EXPECT_EQ(513, converted[0]);
  EXPECT_EQ(514, converted[1]);
  EXPECT_EQ(515, converted[2]);
  EXPECT_EQ(516, converted[3]);
}
