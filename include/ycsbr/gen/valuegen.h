#pragma once

#include <cassert>
#include <cstring>
#include <memory>

#include "ycsbr/gen/types.h"
#include "ycsbr/impl/util.h"

namespace ycsbr {
namespace gen {

class ValueGenerator {
 public:
  ValueGenerator(const size_t value_size, const size_t num_values, PRNG& prng)
      : raw_values_(nullptr),
        value_size_(value_size),
        total_size_(value_size * num_values),
        next_value_index_(0) {
    assert(num_values >= 1);
    assert(value_size_ >= sizeof(uint32_t));
    raw_values_ = impl::GetRandomBytes(total_size_, prng);    //1024个不同的value
    lastindex = total_size_ - value_size_;   ///////////////////////
  }

  const char* NextValue() {    //1024个value循环出现
    const char* to_return = &(raw_values_[next_value_index_]);
    next_value_index_ += value_size_;
    if (next_value_index_ >= total_size_-value_size_) {    //*将最后一个value用作墓碑值   /////////////////////////////
      next_value_index_ = 0;
    }
    return to_return;
  }

  ////////////////////////////////
  const char* LastValue() {    //获取墓碑值
    const char* to_return = &(raw_values_[lastindex]);
    return to_return;
  }
  ////////////////////////////////

  size_t value_size() const { return value_size_; }

 private:
  std::unique_ptr<char[]> raw_values_;
  size_t value_size_;
  size_t total_size_;
  size_t next_value_index_;
  size_t lastindex;   ///////////////////
};

}  // namespace gen
}  // namespace ycsbr
