#include "converter.hpp"

#include <cassert>
#include <algorithm>
#include <execution>

#include <fmt/core.h>

namespace sdc {

Converter::Converter(const parameters& g, const parameters& p)
    : converted(g[0].size())
    , calculations(g[0].size())
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
  clear_previous_data();
  test_data_size_consistency(data);
  return convert(data);
}

std::span<float> Converter::convert(std::span<const uint16_t> data)
{
  using namespace std;
  for (auto i = 0u; i < N_GAINS; i++) {
    transform(std::execution::par_unseq, begin(data), end(data), begin(gains_and_pedestals[i]),
              begin(calculations), [i](auto data, auto g_p) {
                return (i == data >> 14) * ((data & 0x3FFF) - g_p.second) * g_p.first;
              });

    transform(std::execution::par_unseq, begin(calculations), end(calculations), begin(converted),
              begin(converted), std::plus{});
  }
  return converted;
}

void Converter::clear_previous_data()
{
  std::fill(std::execution::par_unseq, std::begin(converted), std::end(converted), 0u);
}

void Converter::test_data_size_consistency(std::span<const uint16_t> data) const
{
  if (data.size() != gains_and_pedestals[0].size())
    throw std::invalid_argument(
        fmt::format("data size doesn't match gains/pedestal arrays size - (expected {} != {})",
                    gains_and_pedestals.size(), data.size()));
}

} // namespace sdc
