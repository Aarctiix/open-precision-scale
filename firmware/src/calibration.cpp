#include "calibration.h"
#include <Preferences.h>

bool Calibration::add(float raw, float grams) {
  if (n_ >= MAX_POINTS || grams <= 0 || raw == 0) return false;
  // keep points sorted by raw value
  uint8_t i = n_;
  while (i > 0 && pts_[i - 1].raw > raw) { pts_[i] = pts_[i - 1]; i--; }
  pts_[i] = {raw, grams};
  n_++;
  return true;
}

float Calibration::toGrams(float raw) const {
  if (n_ == 0) return raw;  // uncalibrated, show raw counts
  // Origin (0,0) is implicit since we always tare first.
  float x0 = 0, y0 = 0;
  for (uint8_t i = 0; i < n_; i++) {
    if (raw <= pts_[i].raw || i == n_ - 1) {
      float x1 = pts_[i].raw, y1 = pts_[i].grams;
      if (raw > x1 && i > 0) { x0 = pts_[i - 1].raw; y0 = pts_[i - 1].grams; }
      return y0 + (raw - x0) * (y1 - y0) / (x1 - x0);
    }
    x0 = pts_[i].raw;
    y0 = pts_[i].grams;
  }
  return raw;
}

bool Calibration::load() {
  Preferences p;
  p.begin("scale", true);
  n_ = p.getUChar("n", 0);
  if (n_ > MAX_POINTS) n_ = 0;
  if (n_) p.getBytes("pts", pts_, sizeof(CalPoint) * n_);
  p.end();
  return n_ > 0;
}

void Calibration::save() const {
  Preferences p;
  p.begin("scale", false);
  p.putUChar("n", n_);
  p.putBytes("pts", pts_, sizeof(CalPoint) * n_);
  p.end();
}
