#include "shared_count_base.hpp"

namespace cpplearn::memory
{
template <typename Ptr>
class CountPtr : final ff::experiments::SharedCountBase
{
  public:

    explicit CountPtr(Ptr p) : ptr_(p) {}

    // To delete the resource
    virtual void dispose() noexcept {
      delete ptr_;
    }
  
    // To delete the control block
    virtual void destroy() noexcept
    {
      delete this;
    }

    CountPtr(const CountPtr&) = delete;
    CountPtr& operator=(const CountPtr& ) = delete;

  private:
    Ptr* ptr_;
};
}

