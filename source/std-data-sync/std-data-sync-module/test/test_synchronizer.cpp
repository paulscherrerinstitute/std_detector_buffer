/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"

#include <gtest/gtest.h>
#include "detectors/common.hpp"


struct SynchronizerWithoutModuleMapTest : public ::testing::Test
{
  static const auto modules = 2u;
  static const auto queue_size = 3u;
  Synchronizer<CommonFrame> sync{modules, queue_size};
};

TEST_F(SynchronizerWithoutModuleMapTest, ShouldSyncImageWhenAllModulesAreReceived)
{
  const auto image_id = 1;
  EXPECT_FALSE(sync.pop_next_full_image());

  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 0}));
  EXPECT_FALSE(sync.pop_next_full_image());

  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 1}));
  EXPECT_EQ(image_id, sync.pop_next_full_image()->image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldDropImageWhenTwiceReceivingSameModule)
{
  const auto image_id = 33;
  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 0}));
  EXPECT_EQ(1, sync.process_image_metadata({image_id, 0, 0}));
  EXPECT_FALSE(sync.pop_next_full_image());
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldDropImageWhenQueueIsFull)
{
  for (auto id = 0u; id < queue_size; id++)
    EXPECT_EQ(0, sync.process_image_metadata({id, 0, 0}));

  EXPECT_EQ(1, sync.process_image_metadata({42, 0, 0}));
}

TEST_F(SynchronizerWithoutModuleMapTest, ShouldReturnImagesAlwaysInOrder)
{
  for (auto id = 0u; id < queue_size; id++)
    EXPECT_EQ(0, sync.process_image_metadata({id, 0, 0}));

  EXPECT_EQ(0, sync.process_image_metadata({2, 0, 1}));
  EXPECT_EQ(0, sync.process_image_metadata({1, 0, 1}));
  EXPECT_FALSE(sync.pop_next_full_image());

  EXPECT_EQ(0, sync.process_image_metadata({0, 0, 1}));
  for (auto id = 0u; id < queue_size; id++)
    EXPECT_EQ(id, sync.pop_next_full_image()->image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}

struct SynchronizerWithModuleMapTest : public ::testing::Test
{
  static const auto modules = 8u;
  static const auto queue_size = 3u;
  Synchronizer<CommonFrame> sync{modules, queue_size, std::bitset<128>("101101")};
};

TEST_F(SynchronizerWithModuleMapTest, ShouldSyncImageWhenAllModulesAreReceived)
{
  const auto image_id = 1;
  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 0}));
  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 2}));
  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 3}));
  EXPECT_FALSE(sync.pop_next_full_image());

  EXPECT_EQ(0, sync.process_image_metadata({image_id, 0, 5}));
  EXPECT_EQ(image_id, sync.pop_next_full_image()->image_id);
  EXPECT_FALSE(sync.pop_next_full_image());
}
