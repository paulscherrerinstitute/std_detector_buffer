#include "gigafrost.hpp"

#include <gtest/gtest.h>

#define N_CACHE_LINE_BYTES 64

TEST(Gigafrost, struct_size)
{
  EXPECT_EQ(sizeof(GigafrostUdpPacket), BYTES_PER_PACKET);
  EXPECT_EQ(sizeof(GigafrostFrame), N_CACHE_LINE_BYTES);
}