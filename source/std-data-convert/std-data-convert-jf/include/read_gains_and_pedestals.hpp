/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <string>

#include "parameters.hpp"

namespace jf::sdc {
std::tuple<parameters, parameters> read_gains_and_pedestals(const std::string& filename,
                                                            std::size_t image_size);
}
