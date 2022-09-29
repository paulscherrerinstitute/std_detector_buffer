/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP
#define STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP

#include <tuple>
#include <string>

#include "parameters.hpp"

namespace sdc {
std::tuple<parameters, parameters> read_gains_and_pedestals(const std::string& filename,
                                                            std::size_t image_size);
}

#endif // STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP
