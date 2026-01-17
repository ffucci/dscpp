#include <vector>

template <typename T>
class SimpleRingBufferFS {
public:
  


private:
  std::atomic<size_t> read_index_{0};
  std::atomic<size_t> write_index_{0};
  std::vector<T> buffer_;
}
