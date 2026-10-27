#pragma once
#include <stddef.h>
#include <math.h>

// Moving average over a fixed window, with simple outlier rejection:
// a sample that sits too far from the current median gets dropped,
// unless we keep seeing "outliers" (meaning the load really changed).
template <size_t N>
class WeightFilter {
 public:
  explicit WeightFilter(float rejectGrams = 5.0f) : reject_(rejectGrams) {}

  void reset() { count_ = 0; head_ = 0; misses_ = 0; }

  // Returns true if the sample was accepted.
  bool push(float v) {
    if (count_ >= 3 && fabsf(v - median()) > reject_) {
      // Three rejections in a row: the user put something on the pan.
      if (++misses_ < 3) return false;
      reset();
    }
    misses_ = 0;
    buf_[head_] = v;
    head_ = (head_ + 1) % N;
    if (count_ < N) count_++;
    return true;
  }

  float mean() const {
    if (!count_) return 0.0f;
    float s = 0;
    for (size_t i = 0; i < count_; i++) s += buf_[i];
    return s / count_;
  }

  float median() const {
    float tmp[N];
    for (size_t i = 0; i < count_; i++) tmp[i] = buf_[i];
    // insertion sort, N is small
    for (size_t i = 1; i < count_; i++) {
      float k = tmp[i];
      size_t j = i;
      while (j > 0 && tmp[j - 1] > k) { tmp[j] = tmp[j - 1]; j--; }
      tmp[j] = k;
    }
    return count_ ? tmp[count_ / 2] : 0.0f;
  }

  // Peak-to-peak spread of the window, used for stability detection.
  float spread() const {
    if (!count_) return 0.0f;
    float lo = buf_[0], hi = buf_[0];
    for (size_t i = 1; i < count_; i++) {
      if (buf_[i] < lo) lo = buf_[i];
      if (buf_[i] > hi) hi = buf_[i];
    }
    return hi - lo;
  }

  bool full() const { return count_ == N; }

 private:
  float buf_[N] = {0};
  size_t count_ = 0, head_ = 0;
  int misses_ = 0;
  float reject_;
};
