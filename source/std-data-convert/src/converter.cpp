#include "converter.hpp"

#include <cassert>
#include <algorithm>

#include <fmt/core.h>

namespace sdc {

Converter::Converter(const parameters& g, const parameters& p)
    : converted(g[0].size())
{
  assert(g[0].size() == p[0].size());

  for (auto i = 0u; i < N_GAINS; i++) {
    gains_and_pedestals[i].reserve(g[i].size());
    std::ranges::transform(g[i], p[i], std::back_inserter(gains_and_pedestals[i]),
                           [](auto a, auto b) { return std::make_pair(a, b); });
  }
}

std::span<float> Converter::convert_data(std::span<const uint16_t> data)
{
  test_data_size_consistency(data);
  return convert(data);
}

std::span<float> Converter::convert(std::span<const uint16_t> data)
{
  for (auto i = 0u; i < data.size(); i++) {
    auto gain_group = data[i] >> 14;
    converted[i] = ((data[i] & 0x3FFF) - gains_and_pedestals[gain_group][i].second) *
                   gains_and_pedestals[gain_group][i].first;
  }
  return converted;
}

void Converter::test_data_size_consistency(std::span<const uint16_t> data) const
{
  if (data.size() != gains_and_pedestals[0].size())
    throw std::invalid_argument(
        fmt::format("data size doesn't match gains/pedestal arrays size - (expected {} != {})",
                    gains_and_pedestals.size(), data.size()));
}

} // namespace sdc
