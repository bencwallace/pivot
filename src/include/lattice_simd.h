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

inline __m128i permutevar_epi32(__m128i data, __m128i perm) {
  return _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(data), perm));
}

// Given a vector (a, b, c, d) representing potentially unsorted intervals
// {a, c} and {b, d}, return [min(a, c), min(b, d), max(a, c), max(b, d)]
// representing intervals [min(a, c), max(a, c)] and [min(b, d), max(b, d)].
inline __m128i sort_bounds(__m128i pairs) {
  __m128i swapped = _mm_shuffle_epi32(pairs, _MM_SHUFFLE(1, 0, 3, 2));
  __m128i minima = _mm_min_epi32(pairs, swapped);
  __m128i maxima = _mm_max_epi32(pairs, swapped);
  return _mm_blend_epi32(minima, maxima, 0b1100);
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

  __m128i data() const { return data_; }

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

template <> class transform<2, true> {

public:
  transform();

  transform(const std::array<int, 2> &perm, const std::array<int, 2> &signs);

  transform(const point<2, true> &p, const point<2, true> &q);

  template <typename Gen> static transform rand(Gen &gen) {
    static std::bernoulli_distribution flip_;

    std::array<int, 2> perm;
    std::array<int, 2> signs;
    for (int i = 0; i < 2; ++i) {
      perm[i] = i;
      signs[i] = 2 * flip_(gen) - 1;
    }
    std::shuffle(perm.begin(), perm.end(), gen);
    return transform(perm, signs);
  }

  static transform rand();

  bool operator==(const transform &t) const;

  point<2, true> operator*(const point<2, true> &p) const;

  transform operator*(const transform &t) const;

  box<2, true> operator*(const box<2, true> &b) const;

  bool is_identity() const;

  transform inverse() const;

  std::array<std::array<int, 2>, 2> to_matrix() const;

  std::string to_string() const;

private:
  __m128i perm_;
  __m128i signs_;

  transform(__m128i perm, __m128i signs) : perm_(perm), signs_(signs) {}
};

} // namespace pivot
