#include "jungfrau.hpp"
#include <gtest/gtest.h>

#define N_CACHE_LINE_BYTES 64

TEST(Jungfrau, struct_size)
{
  EXPECT_EQ(sizeof(JFUdpPacket), BYTES_PER_PACKET);
  EXPECT_EQ(sizeof(JFFrame), N_CACHE_LINE_BYTES);
}