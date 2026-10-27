#pragma once
#include <stdint.h>

// Piecewise-linear calibration. Each point maps a raw (tared) HX711 reading
// to a known mass. With one point it's a plain scale factor; more points
// help with load cells that aren't perfectly linear over their range.
struct CalPoint {
  float raw;
  float grams;
};

class Calibration {
 public:
  static const uint8_t MAX_POINTS = 5;

  void clear() { n_ = 0; }
  bool add(float raw, float grams);
  float toGrams(float raw) const;
  uint8_t size() const { return n_; }

  bool load();   // from NVS
  void save() const;

 private:
  CalPoint pts_[MAX_POINTS];
  uint8_t n_ = 0;
};
