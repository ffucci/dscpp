#pragma once

#include <memory>

using OldSharedPtr = std::shared_ptr<int>;

namespace ff::experiments {

template <typename T> class SharedPtr {
public:
  void inc() { value_++; }

private:
  int value_;
};

} // namespace ff::experiments