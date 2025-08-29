#include <limits>

#include "lattice2d.h"

static inline int32_t extract_epi32(__m128i data, size_t i) {
  switch (i) {
  case 0:
    return _mm_extract_epi32(data, 0);
  case 1:
    return _mm_extract_epi32(data, 1);
  case 2:
    return _mm_extract_epi32(data, 2);
  case 3:
    return _mm_extract_epi32(data, 3);
  default:
    return 0;
  }
}

static inline __m128i insert_epi32(__m128i data, int32_t val, size_t i) {
  switch (i) {
  case 0:
    return _mm_insert_epi32(data, val, 0);
  case 1:
    return _mm_insert_epi32(data, val, 1);
  case 2:
    return _mm_insert_epi32(data, val, 2);
  case 3:
    return _mm_insert_epi32(data, val, 3);
  default:
    return data;
  }
}

namespace pivot {

point2d::point2d() : data_(_mm_setzero_si128()) {}

point2d::point2d(std::array<int32_t, 2> coords) : data_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

point2d point2d::unit() { return point2d(std::array{1, 0}); }

int32_t point2d::operator[](size_t i) const { return extract_epi32(data_, i); }

bool point2d::operator==(const point2d &other) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(data_, other.data_)) == 0xFFFF;
}

bool point2d::operator!=(const point2d &other) const { return !(*this == other); }

point2d point2d::operator+(const point2d &other) const { return _mm_add_epi32(data_, other.data_); }

point2d point2d::operator-(const point2d &other) const { return _mm_sub_epi32(data_, other.data_); }

point2d &point2d::operator*=(int32_t k) {
  data_ = _mm_mullo_epi32(_mm_set1_epi32(k), data_);
  return *this;
}

std::string point2d::to_string() const {
  std::string s = "(";
  s += std::to_string(extract_epi32(data_, 0)) + ", ";
  s += std::to_string(extract_epi32(data_, 1)) + ")";
  return s;
}

interval2d::interval2d() : interval2d(0, 0) {}

interval2d::interval2d(int32_t left, int32_t right) : left_(left), right_(right) {}

bool interval2d::operator==(const interval2d &other) const {
  return (left_ == other.left_ && right_ == other.right_) || (empty() && other.empty());
}

bool interval2d::operator!=(const interval2d &other) const { return !(*this == other); }

bool interval2d::empty() const { return left_ > right_; }

/** @brief Returns the string of the form "[{left_}, {right_}]". */
std::string interval2d::to_string() const {
  // for some reason, gcc-12 gives -Wrestrict warning if I use the string concatenation operator
  std::string result = "[";
  result.append(std::to_string(left_)).append(", ").append(std::to_string(right_)).append("]");
  return result;
}

box2d &box2d::operator+=(const point2d &p) {
  __m128i offset = _mm_shuffle_epi32(p.data(), _MM_SHUFFLE(1, 0, 1, 0));
  data_ = _mm_add_epi32(data_, offset);
  return *this;
}

box2d &box2d::operator-=(const point2d &b) {
  __m128i offset = _mm_shuffle_epi32(b.data(), _MM_SHUFFLE(1, 0, 1, 0));
  data_ = _mm_sub_epi32(data_, offset);
  return *this;
}

box2d::box2d(const std::array<interval2d, 2> &intervals)
    : data_(_mm_setr_epi32(intervals[0].left_, intervals[1].left_, intervals[0].right_, intervals[1].right_)) {}

box2d::box2d(std::span<const point2d> points) {
  std::array<int, 2> min;
  std::array<int, 2> max;
  min.fill(std::numeric_limits<int32_t>::max());
  max.fill(std::numeric_limits<int32_t>::min());
  for (const auto &p : points) {
    for (int i = 0; i < 2; ++i) { // TODO: vectorize (not urgent)
      min[i] = std::min(min[i], p[i]);
      max[i] = std::max(max[i], p[i]);
    }
  }
  // anchor at (1, 0, ..., 0)
  data_ = insert_epi32(data_, min[0] - points[0][0] + 1, 0);
  data_ = insert_epi32(data_, max[0] - points[0][0] + 1, 2);
  data_ = insert_epi32(data_, min[1] - points[0][1], 1);
  data_ = insert_epi32(data_, max[1] - points[0][1], 3);
}

bool box2d::operator==(const box2d &b) const { return _mm_movemask_epi8(_mm_cmpeq_epi32(data_, b.data_)) == 0xFFFF; }

bool box2d::operator!=(const box2d &b) const { return !(*this == b); }

interval2d box2d::operator[](int i) const {
  return {int32_t(extract_epi32(data_, i)), int32_t(extract_epi32(data_, i + 2))};
}

bool box2d::empty() const {
  __m128i swapped = _mm_shuffle_epi32(data_, _MM_SHUFFLE(1, 0, 3, 2));
  __m128i cmp = _mm_cmpgt_epi32(data_, swapped);
  auto result = _mm_cvtsi128_si64(cmp); // Only check the lower 64 bits
  return result != 0;
}

box2d box2d::operator|(const box2d &b) const {
  // TODO: Look into using `_mm_maskz_{min,max}_epi32`
  // For example: `__m128i mins = _mm_min_epi32(0b0011, data, b.data)`
  // For now, benchmarks are crashing when I enable AVX512 in the build
  __m128i mins = _mm_min_epi32(data_, b.data_);
  __m128i maxs = _mm_max_epi32(data_, b.data_);
  return _mm_blend_epi32(mins, maxs, 0b1100);
}

box2d box2d::operator&(const box2d &b) const {
  // TODO: See comment under `operator|`
  __m128i mins = _mm_min_epi32(data_, b.data_);
  __m128i maxs = _mm_max_epi32(data_, b.data_);
  return _mm_blend_epi32(mins, maxs, 0b0011);
}

std::string box2d::to_string() const {
  std::string s = "";
  s += (*this)[0].to_string() + " x " + (*this)[1].to_string();
  return s;
}

} // namespace pivot
