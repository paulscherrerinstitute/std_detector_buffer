#include "converter.hpp"
#include <fmt/core.h>

namespace sdc {

Converter::Converter(parameters g, parameters p)
    : gains(std::move(g))
    , pedestals(std::move(p))
    , converted(gains[0].size())
{}

std::span<float> Converter::convert_data(std::span<const uint16_t> data)
{
  test_data_size_consistency(data);
  return converted;
}

void Converter::test_data_size_consistency(std::span<const uint16_t> data) const
{
  if (data.size() != gains.size())
    throw std::invalid_argument(
        fmt::format("data size doesn't match gains/pedestal arrays size - (expected {} != {})",
                    gains.size(), data.size()));
}

} // namespace sdc
