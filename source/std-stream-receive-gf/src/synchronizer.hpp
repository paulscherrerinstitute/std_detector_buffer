/////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Paul Scherrer Institute. All rights reserved.
/////////////////////////////////////////////////////////////////////

#ifndef STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
#define STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP

#include <deque>
#include <algorithm>

namespace ssrg {

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
      if (sync.size() >= drop) sync.pop_front();
      sync.push_back(id);
      return false;
    }
  }

private:
  std::deque<uint64_t> sync;
  std::size_t drop;
};

} // namespace ssrg

#endif // STD_DETECTOR_BUFFER_SYNCHRONIZER_HPP
