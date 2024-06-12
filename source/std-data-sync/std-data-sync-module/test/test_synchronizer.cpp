/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <gtest/gtest.h>
#include "detectors/gigafrost.hpp"

struct SynchronizerWithoutModuleMapTest : public ::testing::Test
{
  static const auto modules = 2u;
  static const auto queue_size = 3u;
  Synchronizer<gf::GFFrame> sync{modules, queue_size};
};

TEST_F(SynchronizerWithoutModuleMapTest, ShouldSyncImageWhenAllModulesAreReceived)
{
  const auto image_id = 1;
  EXPECT_FALSE(sync.pop_next_full_image());
  gf::GFFrame frame{};
  frame.common.image_id = image_id;

  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_FALSE(sync.pop_next_full_image());

  frame.common.module_id = 1;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_EQ(image_id, sync.pop_next_full_image()->common.image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldDropImageWhenTwiceReceivingSameModule)
{
  gf::GFFrame frame{};
  frame.common.image_id = 33;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_EQ(1, sync.process_image_metadata(frame));
  EXPECT_FALSE(sync.pop_next_full_image());
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldDropImageWhenQueueIsFull)
{
  gf::GFFrame frame{};
  for (auto id = 0u; id < queue_size; id++) {
    frame.common.image_id = id;
    EXPECT_EQ(0, sync.process_image_metadata(frame));
  }
  frame.common.image_id = 42;
  EXPECT_EQ(1, sync.process_image_metadata(frame));
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldReturnImagesAlwaysInOrder)
{
  gf::GFFrame frame{};
  for (auto id = 0u; id < queue_size; id++) {
    frame.common.image_id = id;
    EXPECT_EQ(0, sync.process_image_metadata(frame));
  }
  frame.common.image_id = 2;
  frame.common.module_id = 1;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  frame.common.image_id = 1;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_FALSE(sync.pop_next_full_image());

  frame.common.image_id = 0;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  for (auto id = 0u; id < queue_size; id++)
    EXPECT_EQ(id, sync.pop_next_full_image()->common.image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}

struct SynchronizerWithModuleMapTest : public ::testing::Test
{
  static const auto modules = 8u;
  static const auto queue_size = 3u;
  Synchronizer<gf::GFFrame> sync{modules, queue_size, std::bitset<128>("101101")};
};

TEST_F(SynchronizerWithModuleMapTest, ShouldSyncImageWhenAllModulesAreReceived)
{
  gf::GFFrame frame{};
  frame.common.image_id = 1;
  frame.common.module_id = 0;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  frame.common.module_id = 2;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  frame.common.module_id = 3;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_FALSE(sync.pop_next_full_image());

  frame.common.module_id = 5;
  EXPECT_EQ(0, sync.process_image_metadata(frame));
  EXPECT_EQ(frame.common.image_id, sync.pop_next_full_image()->common.image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}
