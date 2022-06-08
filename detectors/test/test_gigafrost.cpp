#include "gigafrost.hpp"

#include <gtest/gtest.h>

#define N_CACHE_LINE_BYTES 64

TEST(Gigafrost, struct_size)
{
    EXPECT_EQ(sizeof(det_packet), BYTES_PER_PACKET);
    EXPECT_EQ(sizeof(frame_metadata), N_CACHE_LINE_BYTES);
}