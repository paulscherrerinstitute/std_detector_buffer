/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <deque>
#include <algorithm>

namespace gf::rec {

class Synchronizer
{
public:
  explicit Synchronizer(std::size_t drop_after)
      : drop(drop_after)
  {}

  bool is_ready_to_send(uint64_t id)
  {
    if (auto i = std::ranges::find(sync, id); i != std::end(sync)) {
      sync.erase(i);
      return true;
    }
    else {
      if (sync.size() >= drop) {
        sync.pop_front();
        dropped_packages++;
      }
      sync.push_back(id);
      return false;
    }
  }

  [[nodiscard]] unsigned long get_dropped_packages() const { return dropped_packages; }
  void reset_dropped_packages() { dropped_packages = 0; }

private:
  std::deque<uint64_t> sync;
  std::size_t drop;
  unsigned long dropped_packages = 0;
};

} // namespace gf::rec

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
