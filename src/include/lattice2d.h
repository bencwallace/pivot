#pragma once

#include <array>
#include <immintrin.h>
#include <span>

#include <boost/operators.hpp>

static inline int32_t extract_epi32(__m128i data, size_t i);

static inline __m128i insert_epi32(__m128i data, int32_t val, size_t i);

namespace pivot {

class point2d : boost::multipliable<point2d, int32_t> {

public:
  point2d();

  point2d(std::array<int32_t, 2> coords);

  static point2d unit();

  __m128i data() const { return data_; }

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

struct interval2d {
  int32_t left_;
  int32_t right_;

  interval2d();
  interval2d(int32_t left, int32_t right);

  bool operator==(const interval2d &i) const;

  bool operator!=(const interval2d &i) const;

  bool empty() const;

  std::string to_string() const;
};

struct box2d : boost::additive<box2d, point2d> {
  __m128i data_; // x_min, y_min, x_max, y_max

  box2d() = delete;

  box2d(const std::array<interval2d, 2> &intervals);

  box2d(std::span<const point2d> points);

  bool operator==(const box2d &b) const;

  bool operator!=(const box2d &b) const;

  interval2d operator[](int i) const;

  bool empty() const;

  box2d &operator+=(const point2d &b);

  box2d &operator-=(const point2d &b);

  box2d operator|(const box2d &b) const;

  box2d operator&(const box2d &b) const;

  std::string to_string() const;

private:
  box2d(__m128i data) : data_(data) {}
};

} // namespace pivot
