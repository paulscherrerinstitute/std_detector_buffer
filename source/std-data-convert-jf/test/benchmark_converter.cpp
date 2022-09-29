/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#include "converter.hpp"

#include <benchmark/benchmark.h>
#include <range/v3/all.hpp>

using namespace ranges;

namespace {
constexpr std::size_t data_elements = 512 * 1024;

sdc::jf::parameters prepare_params(float default_value = 0)
{
  return sdc::jf::parameters{std::vector(data_elements, default_value),
                         std::vector(data_elements, default_value),
                         std::vector(data_elements, default_value)};
}

} // namespace

static void BM_Convertion(benchmark::State& state)
{
  auto input_data = std::vector<uint16_t>(data_elements, 0);
  sdc::jf::Converter converter{prepare_params(2), prepare_params(-1)};
  for (auto _ : state) {
    state.PauseTiming();
    ranges::transform(input_data, begin(input_data),
                      [](auto) { return std::rand() % 0x3FF + (std::rand() % 4 << 14); });
    state.ResumeTiming();
    benchmark::DoNotOptimize(converter.convert_data(input_data));
  }
}
BENCHMARK(BM_Convertion);

BENCHMARK_MAIN();
