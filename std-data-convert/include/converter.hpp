#ifndef STD_DETECTOR_BUFFER_CONVERTER_HPP
#define STD_DETECTOR_BUFFER_CONVERTER_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <span>

namespace sdc {

static_assert(sizeof(float) == 4u, "float must comply to IEEE-754 standard for floating points");

constexpr inline std::size_t N_GAINS = 4u;
using parameters = std::array<std::vector<float>, N_GAINS>;
using parameters_pairs = std::array<std::vector<std::pair<float, float>>, N_GAINS>;

class Converter
{
public:
  explicit Converter(const parameters& g, const parameters& p);
  std::span<float> convert_data(std::span<const uint16_t> data);

private:
  void test_data_size_consistency(std::span<const uint16_t> data) const;
  std::span<float> convert(std::span<const uint16_t> data);

  parameters_pairs gains_and_pedestals;
  std::vector<float> converted;
};

} // namespace sdc

#endif // STD_DETECTOR_BUFFER_CONVERTER_HPP
