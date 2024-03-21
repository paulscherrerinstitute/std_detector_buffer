/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <gtest/gtest.h>
#include <range/v3/all.hpp>

#include "detectors/jungfrau.hpp"

using namespace ranges;

namespace {
constexpr std::size_t data_elements = MODULE_X_SIZE * MODULE_Y_SIZE;

const auto iota_data = iota_view<uint16_t>(0) |
                       views::transform([](auto a) -> uint16_t { return a & 0x3FF; }) |
                       views::take(data_elements) | to_vector;

jf::sdc::parameters prepare_params(float default_value = 0)
{
  return jf::sdc::parameters{std::vector(data_elements, default_value),
                             std::vector(data_elements, default_value),
                             std::vector(data_elements, default_value)};
}

utils::DetectorConfig config{"jf",
                             "jungfrau-converted",
                             4,
                             16,
                             2 * MODULE_Y_SIZE,
                             2 * MODULE_X_SIZE,
                             0,
                             0,
                             "debug",
                             std::chrono::seconds(30),
                             8,
                             false,
                             16777216,
                             false,
                             50,
                             {{0, {{0, 0}, {1023, 511}}},
                              {1, {{0, 512}, {1023, 1023}}},
                              {2, {{1024, 0}, {2047, 511}}},
                              {3, {{1024, 512}, {2047, 1023}}}}};

} // namespace

TEST(ConverterJf, ShouldThrowWhenDataSizeIsInconsistentWithConfiguredGains)
{
  jf::sdc::Converter converter{prepare_params(), prepare_params(), config, 0};
  std::vector<float> output(config.image_pixel_width * config.image_pixel_height);
  uint16_t invalid_data[MODULE_X_SIZE * MODULE_Y_SIZE * 4 + 1] = {};
  std::span output_as_uints{(uint16_t*)output.data(), output.size() / 2};
  EXPECT_THROW(converter.convert(invalid_data, output_as_uints), std::invalid_argument);
}

TEST(ConverterJf, ShouldReturnSameOutputAsInputDataWhenPedestalIs0AndGain1)
{
  jf::sdc::Converter converter{prepare_params(1), prepare_params(0), config, 0};
  std::vector<float> output(config.image_pixel_width * config.image_pixel_height);
  std::span output_as_uints{(uint16_t*)output.data(), output.size() / 2};

  converter.convert(iota_data, output_as_uints);
  for (auto i = 0u; i < MODULE_Y_SIZE; i++) {
    const auto expected_line =
        iota_data | views::drop(i * MODULE_X_SIZE) | views::take(MODULE_X_SIZE);
    const auto output_line =
        output | views::drop(i * MODULE_X_SIZE * 2) | views::take(MODULE_X_SIZE);
    EXPECT_TRUE(equal(expected_line, output_line));
  }
}

TEST(ConverterJf, ShouldMultiplyTheInputDataViaGain)
{
  jf::sdc::Converter converter{prepare_params(2), prepare_params(0), config, 0};
  std::vector<float> output(config.image_pixel_width * config.image_pixel_height);
  std::span output_as_uints{(uint16_t*)output.data(), output.size() / 2};

  converter.convert(iota_data, output_as_uints);
  for (auto i = 0u; i < MODULE_Y_SIZE; i++) {
    const auto expected_line = iota_data | views::drop(i * MODULE_X_SIZE) |
                               views::take(MODULE_X_SIZE) |
                               views::transform([](auto a) { return 2.f * a; });
    const auto output_line =
        output | views::drop(i * MODULE_X_SIZE * 2) | views::take(MODULE_X_SIZE);
    EXPECT_TRUE(equal(expected_line, output_line));
  }
}

TEST(ConverterJf, ShouldCalculateValuesUsingGainAndPedestal)
{
  jf::sdc::Converter converter{prepare_params(2), prepare_params(-1), config, 0};
  std::vector<float> output(config.image_pixel_width * config.image_pixel_height);
  std::span output_as_uints{(uint16_t*)output.data(), output.size() / 2};

  converter.convert(iota_data, output_as_uints);
  for (auto i = 0u; i < MODULE_Y_SIZE; i++) {
    const auto expected_line = iota_data | views::drop(i * MODULE_X_SIZE) |
                               views::take(MODULE_X_SIZE) |
                               views::transform([](auto a) { return 2.f * (a + 1); });
    const auto output_line =
        output | views::drop(i * MODULE_X_SIZE * 2) | views::take(MODULE_X_SIZE);
    EXPECT_TRUE(equal(expected_line, output_line));
  }
}

TEST(ConverterJf, ShouldCalculateValuesUsingGainAndPedestalForModule3)
{
  jf::sdc::Converter converter{prepare_params(3), prepare_params(-1), config, 3};
  std::vector<float> output(config.image_pixel_width * config.image_pixel_height);
  std::span output_as_uints{(uint16_t*)output.data(), output.size() / 2};

  converter.convert(iota_data, output_as_uints);
  for (auto i = 0u; i < MODULE_Y_SIZE; i++) {
    const auto expected_line = iota_data | views::drop(i * MODULE_X_SIZE) |
                               views::take(MODULE_X_SIZE) |
                               views::transform([](auto a) { return 3.f * (a + 1); });
    const auto output_line = output |
                             views::drop((MODULE_Y_SIZE + i) * MODULE_X_SIZE * 2 + MODULE_X_SIZE) |
                             views::take(MODULE_X_SIZE);
    EXPECT_TRUE(equal(expected_line, output_line));
  }
}
