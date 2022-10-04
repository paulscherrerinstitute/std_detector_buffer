/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_IDENTIFIER_HPP
#define STD_DETECTOR_BUFFER_IDENTIFIER_HPP

#include <string_view>
#include <fmt/core.h>

namespace jf::sdc {
class Identifier
{
public:
  explicit Identifier(std::string_view name, uint16_t module, uint16_t converter_id)
      : repr(fmt::format("{}-{}-converted-{}", name, module, converter_id))
      , source(fmt::format("{}-{}", name, module))
  {}
  [[nodiscard]] std::string converter_name() const { return repr; }
  [[nodiscard]] std::string source_name() const { return source; }

private:
  std::string repr;
  std::string source;
};
} // namespace jf::sdc

#endif // STD_DETECTOR_BUFFER_IDENTIFIER_HPP
