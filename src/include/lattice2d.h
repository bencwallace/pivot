#pragma once

#include <array>
#include <immintrin.h>

#include <boost/operators.hpp>

static inline int32_t extract_epi32(__m128i data, size_t i);

static inline __m128i insert_epi32(__m128i data, int32_t val, size_t i);

namespace pivot {

class point2d : boost::multipliable<point2d, int32_t> {

public:
  point2d();

  point2d(std::array<int32_t, 2> coords);

  static point2d unit();

  int32_t operator[](size_t i) const;

  bool operator==(const point2d &other) const;

  bool operator!=(const point2d &other) const;

  point2d operator+(const point2d &other) const;

  point2d operator-(const point2d &other) const;

  point2d &operator*=(int32_t k);

  std::string to_string() const;

private:
  __m128i data_;

  point2d(__m128i data) : data_(data) {}
};

} // namespace pivot
