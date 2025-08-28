#include <immintrin.h>

#include "lattice.h"

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

static inline __m128i insert_epi32(__m128i data, int val, size_t i) {
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

template <> class point<2> : boost::multipliable<point<2>, int> {

public:
  point() : coords_(_mm_setzero_si128()) {}

  point(const std::array<int, 2> &coords) : coords_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

  static point unit(int i) {
    __m128i coords = _mm_setzero_si128();
    coords = insert_epi32(coords, 1, i);
    return point(coords);
  }

  int operator[](int i) const { return extract_epi32(coords_, i); }

  bool operator==(const point &p) const { return _mm_movemask_epi8(_mm_cmpeq_epi32(coords_, p.coords_)) == 0xFFFF; }

  bool operator!=(const point &p) const { return !(*this == p); }

  point operator+(const point &p) const { return _mm_add_epi32(coords_, p.coords_); }

  point operator-(const point &p) const { return _mm_sub_epi32(coords_, p.coords_); }

  point &operator*=(int k) {
    coords_ = _mm_mullo_epi32(coords_, _mm_set1_epi32(k));
    return *this;
  }

  __m128i coords() const { return coords_; }

  // TODO: double-check
  int norm() const {
    __m128i squared = _mm_mullo_epi32(coords_, coords_);
    return extract_epi32(_mm_hadd_epi32(squared, squared), 0);
  }

  std::string to_string() const {
    std::string s = "(";
    s += std::to_string(extract_epi32(coords_, 0)) + ", ";
    s += std::to_string(extract_epi32(coords_, 1));
    s += ")";
    return s;
  }

private:
  __m128i coords_;

  point(__m128i coords) : coords_(coords) {}
};

template <> struct box<2> : boost::additive<box<2>, point<2>> {
  box() = delete;

  box(const std::array<interval, 2> &intervals)
      : data_(_mm_setr_epi32(intervals[0].left_, intervals[1].left_, intervals[0].right_, intervals[1].right_)) {}

  box(std::span<const point<2>> points) {
    std::array<int, 2> min;
    std::array<int, 2> max;
    min.fill(std::numeric_limits<int>::max());
    max.fill(std::numeric_limits<int>::min());
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

  bool operator==(const box &b) const { return _mm_movemask_epi8(_mm_cmpeq_epi32(data_, b.data_)) == 0xFFFF; }

  bool operator!=(const box &b) const { return !(*this == b); }

  interval operator[](int i) const { return {int32_t(extract_epi32(data_, i)), int32_t(extract_epi32(data_, i + 2))}; }

  bool empty() const {
    __m128i swapped = _mm_shuffle_epi32(data_, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i cmp = _mm_cmpgt_epi32(data_, swapped);
    auto result = _mm_cvtsi128_si64(cmp); // Only check the lower 64 bits
    return result != 0;
  }

  box<2> &operator+=(const point<2> &b) {
    __m128i offset = _mm_shuffle_epi32(b.coords(), _MM_SHUFFLE(1, 0, 1, 0));
    data_ = _mm_add_epi32(data_, offset);
    return *this;
  }

  box<2> &operator-=(const point<2> &b) {
    __m128i offset = _mm_shuffle_epi32(b.coords(), _MM_SHUFFLE(1, 0, 1, 0));
    data_ = _mm_sub_epi32(data_, offset);
    return *this;
  }

  box operator|(const box &b) const {
    // TODO: Look into using `_mm_maskz_{min,max}_epi32`
    // For example: `__m128i mins = _mm_min_epi32(0b0011, data, b.data)`
    __m128i mins = _mm_min_epi32(data_, b.data_);
    __m128i maxs = _mm_max_epi32(data_, b.data_);
    return _mm_blend_epi32(mins, maxs, 0b1100);
  }

  box operator&(const box &b) const {
    // TODO: See comment under `operator|`
    __m128i mins = _mm_min_epi32(data_, b.data_);
    __m128i maxs = _mm_max_epi32(data_, b.data_);
    return _mm_blend_epi32(mins, maxs, 0b0011);
  }

  std::string to_string() const {
    std::string s = "";
    s += "[" + std::to_string(extract_epi32(data_, 0)) + ", " + std::to_string(extract_epi32(data_, 2)) + "] x ";
    s += "[" + std::to_string(extract_epi32(data_, 1)) + ", " + std::to_string(extract_epi32(data_, 3)) + "]";
    return s;
  }

private:
  __m128i data_; // x_min, y_min, x_max, y_max

  box(__m128i data) : data_(data) {}
};

} // namespace pivot
