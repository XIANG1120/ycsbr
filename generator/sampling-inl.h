#include <cassert>
#include <limits>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace ycsbr {
namespace gen {

template <typename T, class RNG>
inline void FloydSample(const size_t num_samples, const Range<T>& range,
                        std::vector<T>* dest, const size_t start_index,
                        RNG& rng) {
  assert(range.size() >= num_samples);
  assert(start_index < dest->size());
  assert(start_index + num_samples <= dest->size());

  std::unordered_set<T> samples;
  samples.reserve(num_samples);
  for (T curr = range.max() - num_samples + 1; curr <= range.max(); ++curr) {
    std::uniform_int_distribution<T> dist(range.min(), curr);  //?std::uniform_int_distribution用于生成均匀分布的整数随机数。它通常与伪随机数生成器（如 std::mt19937）一起使用，用于生成指定范围内的随机整数。
    const T next = dist(rng);
    auto res = samples.insert(next);
    if (!res.second) {  //如果插入失败（key重复），则插入curr
      samples.insert(curr);
    }
  }
  assert(samples.size() == num_samples);

  // Copy samples into the destination vector.
  size_t i = start_index;
  for (auto val : samples) {
    (*dest)[i++] = val;
  }
}

template <typename T, class RNG>
void SelectionSample(const size_t num_samples, const Range<T>& range,
                     std::vector<T>* dest, const size_t start_index, RNG& rng) {
  assert(range.size() >= num_samples);
  assert(start_index < dest->size());
  assert(start_index + num_samples <= dest->size());

  std::uniform_real_distribution<double> dist(0.0, 1.0);  //?std::uniform_real_distribution<double>生成指定范围内的随机double类型的数
  const T interval = range.max() - range.min() + 1;
  size_t samples_so_far = 0;
  T curr = 0;
  while (samples_so_far < num_samples) {
    const double u = dist(rng);
    if ((interval - curr) * u < num_samples - samples_so_far) {   //(如果range大小-循环次数)*u < 未生成的键的数量
      (*dest)[start_index + samples_so_far++] = range.min() + curr;
    }
    ++curr;
  }
}

template <typename T, class RNG>
void FisherYatesSample(const size_t num_samples, const Range<T>& range,
                       std::vector<T>* dest, const size_t start_index,
                       RNG& rng) {
  assert(range.size() >= num_samples);
  assert(start_index < dest->size());
  assert(start_index + num_samples <= dest->size());

  std::unordered_map<size_t, T> swapped_indices;
  swapped_indices.reserve(num_samples);

  const T interval = range.max() - range.min() + 1;
  for (size_t i = 0; i < num_samples; ++i) {
    std::uniform_int_distribution<size_t> dist(i, interval - 1);
    const size_t to_swap_idx = dist(rng);
    auto it = swapped_indices.find(to_swap_idx);
    if (it == swapped_indices.end()) {
      // That index has not been swapped yet.
      (*dest)[start_index + i] = range.min() + to_swap_idx;
    } else {
      (*dest)[start_index + i] = range.min() + it->second;
    }

    auto curr_it = swapped_indices.find(i);
    if (curr_it == swapped_indices.end()) {
      swapped_indices[to_swap_idx] = i;
    } else {
      swapped_indices[to_swap_idx] = curr_it->second;
    }
  }
}

template <typename T, class RNG>
void SampleWithoutReplacement(const size_t num_samples, const Range<T>& range,
                              std::vector<T>* dest, const size_t start_index,
                              RNG& rng) {
  constexpr double floyd_selectivity_threshold = 0.05;
  assert(range.size() >= num_samples);  //规定的keyrange必须大于等于要生成的key个数
  const size_t interval = range.max() - range.min() + 1;  //规定的keyrange的个数
  const double selectivity = static_cast<double>(num_samples) / interval;
  if (selectivity <= floyd_selectivity_threshold) {   //<=0.05??
    FloydSample<T, RNG>(num_samples, range, dest, start_index, rng);
  } else {
    SelectionSample<T, RNG>(num_samples, range, dest, start_index, rng);
  }
}

}  // namespace gen
}  // namespace ycsbr
