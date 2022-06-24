#ifndef STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP
#define STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP

#include <tuple>
#include <string_view>

#include "parameters.hpp"

namespace sdc {
std::tuple<parameters, parameters> read_gains_and_pedestals(std::string_view filename);
}

#endif // STD_DETECTOR_BUFFER_READ_GAINS_AND_PEDESTALS_HPP
