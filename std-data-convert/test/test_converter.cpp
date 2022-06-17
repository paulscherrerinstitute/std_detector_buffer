#include "converter.hpp"

#include <gtest/gtest.h>

sdc::parameters prepare_gains()
{
  return sdc::parameters{};
}

TEST(Converter, ShouldThrowWhenDataSizeIsInconsistentWithConfiguredGains)
{
  sdc::Converter c(prepare_gains(), prepare_gains());
  uint16_t data[] = {1, 2, 3};
  EXPECT_THROW(c.convert_data(data), std::invalid_argument);
}
