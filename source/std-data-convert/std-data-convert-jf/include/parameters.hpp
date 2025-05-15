/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace jf::sdc {
static_assert(sizeof(float) == 4u, "float must comply to IEEE-754 standard for floating points");

constexpr inline std::size_t N_GAINS = 3u;
using parameters = std::array<std::vector<float>, N_GAINS>;
using parameters_pairs = std::array<std::vector<std::pair<float, float>>, N_GAINS>;
} // namespace jf::sdc
