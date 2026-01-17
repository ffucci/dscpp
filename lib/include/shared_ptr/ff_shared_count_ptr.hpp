#pragma once

#include <cstddef>

namespace ff::experiments {
class SharedCountPtr {
public:
  explicit SharedCountPtr() : use_count_(1), weak_count_(1) {}
  virtual ~SharedCountPtr() noexcept {}

  virtual void destroy() noexcept {
    // Custom destroy logic
    delete this;
  }

  virtual void dispose() noexcept = 0;

  virtual void add_use_count() noexcept { ++use_count_; }

  virtual void add_weak_count() noexcept { ++weak_count_; }

  virtual void release() noexcept {
    if (--use_count_ == 0) {
      dispose();
      if (--weak_count_ == 0) {
        destroy();
      }
    }
  }

  auto get_use_count() const noexcept { return use_count_; }
  auto get_weak_count() const noexcept { return weak_count_; }

private:
  SharedCountPtr(const SharedCountPtr &) = delete;
  SharedCountPtr &operator=(const SharedCountPtr &) = delete;

  // We avoid using std::atomic here to keep the code simple and focus on the
  // shared pointer implementation.
  size_t use_count_;
  size_t weak_count_;
};
} // namespace ff::experiments