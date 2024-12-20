/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "synchronizer.hpp"
#include "detectors/gigafrost.hpp"

#include <benchmark/benchmark.h>

namespace {

const int n_modules = 8;
const int sync_n_images_buffer = 10000;
const int tested_frequency = 2000;

  int correct_stream_sync(Synchronizer<gf::GFFrame>& syncer)
{
  gf::GFFrame meta{};

  for (int i = 0; i < tested_frequency; i++) {
    meta.common.image_id = i;
    for (int i_module = 0; i_module < n_modules; i_module++) {
      meta.common.module_id = i_module;

      auto result = syncer.process_image_metadata(meta);
      (void)result;
    }
  }
  return 0;
}

void GF_sync_normal(benchmark::State& state)
{
  Synchronizer<gf::GFFrame> syncer(n_modules, sync_n_images_buffer);

  for (auto _ : state) {
    benchmark::DoNotOptimize(correct_stream_sync(syncer));
  }
}

int missing_stream_sync(Synchronizer<gf::GFFrame>& syncer)
{
  gf::GFFrame meta{};

  for (int i_frame = 0; i_frame < tested_frequency; i_frame++) {
    meta.common.image_id = i_frame;
    for (int i_module = 0; i_module < n_modules - 1; i_module++) {
      meta.common.module_id = i_module;

      auto result = syncer.process_image_metadata(meta);
      (void)result;
    }
    if (i_frame % 2 == 0) {
      meta.common.module_id = 7;
      auto result = syncer.process_image_metadata(meta);
      (void)result;
    }
  }
  return 0;
}

void GF_sync_missing(benchmark::State& state)
{
  Synchronizer<gf::GFFrame> syncer(n_modules, sync_n_images_buffer);

  for (auto _ : state) {
    benchmark::DoNotOptimize(missing_stream_sync(syncer));
  }
}
} // namespace

BENCHMARK(GF_sync_normal);
BENCHMARK(GF_sync_missing);

BENCHMARK_MAIN();
