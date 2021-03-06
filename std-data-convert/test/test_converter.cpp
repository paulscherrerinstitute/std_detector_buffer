#include "converter.hpp"

#include <gtest/gtest.h>
#include <range/v3/all.hpp>

using namespace ranges;

namespace {
constexpr std::size_t data_elements = 512 * 1024;

const auto iota_data = iota_view<uint16_t>(0) |
                       views::transform([](auto a) -> uint16_t { return a & 0x3FF; }) |
                       views::take(data_elements) | to_vector;

sdc::parameters prepare_params(float default_value = 0)
{
  return sdc::parameters{std::vector(data_elements, default_value),
                         std::vector(data_elements, default_value),
                         std::vector(data_elements, default_value)};
}

} // namespace

TEST(Converter, ShouldThrowWhenDataSizeIsInconsistentWithConfiguredGains)
{
  sdc::Converter converter{prepare_params(), prepare_params()};
  uint16_t invalid_data[] = {1, 2, 3};
  EXPECT_THROW(converter.convert_data(invalid_data), std::invalid_argument);
}

TEST(Converter, ShouldReturnSameOutputAsInputDataWhenPedestalIs0AndGain1)
{
  sdc::Converter converter{prepare_params(1), prepare_params(0)};
  using namespace ranges;

  auto converted = converter.convert_data(iota_data);
  EXPECT_TRUE(equal(iota_data, converted | views::transform(convert_to<uint16_t>{})));
}

TEST(Converter, ShouldMultiplyTheInputDataViaGain)
{
  sdc::Converter converter{prepare_params(2), prepare_params(0)};
  auto converted = converter.convert_data(iota_data);
  EXPECT_TRUE(equal(iota_data | views::transform([](auto a) { return 2.f * a; }), converted));
}

TEST(Converter, ShouldCalculateValuesUsingGainAndPedestal)
{
  sdc::Converter converter{prepare_params(2), prepare_params(-1)};
  auto converted = converter.convert_data(iota_data);
  EXPECT_TRUE(equal(iota_data | views::transform([](auto a) { return 2.f * (a + 1); }), converted));
}
