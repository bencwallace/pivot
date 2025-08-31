#pragma once

#include <immintrin.h>

#include "lattice.h"

static inline uint32_t extract_epi32(__m128i data, size_t i) {
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

template <> class point<2, true> : boost::multipliable<point<2, true>, int> {

public:
  point();

  point(__m128i coords);

  point(const std::array<int, 2> &coords);

  static point unit(int i);

  __m128i data() const { return coords_; }

  int operator[](int i) const;

  bool operator==(const point &p) const;

  bool operator!=(const point &p) const;

  point operator+(const point &p) const;

  point operator-(const point &p) const;

  point &operator*=(int k);

  //   int norm() const;

  std::string to_string() const;

private:
  __m128i coords_;
};

template <> struct box<2, true> : boost::additive<box<2, true>, point<2, true>> {
  __m128i data_; // x_min, y_min, x_max, y_max

  box() = delete;

  box(__m128i data) : data_(data) {}

  box(const std::array<interval, 2> &intervals);

  box(std::span<const point<2, true>> points);

  bool operator==(const box &b) const;

  bool operator!=(const box &b) const;

  interval operator[](int i) const;

  bool empty() const;

  box<2, true> &operator+=(const point<2, true> &b);

  box<2, true> &operator-=(const point<2, true> &b);

  box operator|(const box &b) const;

  box operator&(const box &b) const;

  std::string to_string() const;
};

} // namespace pivot
