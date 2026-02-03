#pragma once

#include <cstddef>

namespace cpplearn::memory {
class SharedCountBase {
public:
  explicit SharedCountBase() : use_count_(1), weak_count_(1) {}
  virtual ~SharedCountBase() noexcept = default; 

  virtual void destroy() noexcept = 0;
  virtual void dispose() noexcept = 0;

  virtual void add_use_count() noexcept { ++use_count_; }
  virtual void add_weak_count() noexcept { ++weak_count_; }
  virtual void release_use_count() noexcept { --use_count_; }
  virtual void release_weak_count() noexcept { --weak_count_; }

  virtual void release() noexcept {
    if (--use_count_ == 0) {
      dispose();
    if (--weak_count_ == 0) {
        destroy();
      }
    }
  }

  constexpr auto use_count() const noexcept { return use_count_; }
  constexpr auto weak_count() const noexcept { return weak_count_; }

private:
  SharedCountBase(const SharedCountBase &) = delete;
  SharedCountBase &operator=(const SharedCountBase &) = delete;

  // We avoid using std::atomic here to keep the code simple and focus on the
  // shared pointer implementation.
  // Todo: add lock policy in the future
  size_t use_count_;
  size_t weak_count_;
};
} // namespace ff::experiments
