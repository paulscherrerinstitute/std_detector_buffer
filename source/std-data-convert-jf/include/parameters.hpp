/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_PARAMETERS_HPP
#define STD_DETECTOR_BUFFER_PARAMETERS_HPP

#include <cstdint>
#include <vector>
#include <array>

namespace sdc {
static_assert(sizeof(float) == 4u, "float must comply to IEEE-754 standard for floating points");

constexpr inline std::size_t N_GAINS = 3u;
using parameters = std::array<std::vector<float>, N_GAINS>;
using parameters_pairs = std::array<std::vector<std::pair<float, float>>, N_GAINS>;
} // namespace sdc

#endif // STD_DETECTOR_BUFFER_PARAMETERS_HPP
