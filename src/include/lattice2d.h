#pragma once

#include <algorithm>
#include <array>
#include <immintrin.h>
#include <random>
#include <span>

#include <boost/operators.hpp>

static inline int32_t extract_epi32(__m128i data, size_t i);

static inline __m128i insert_epi32(__m128i data, int32_t val, size_t i);

inline __m128i sort_bounds(__m128i pairs);

inline __m128i permutevar_epi32(__m128i data, __m128i perm);

namespace pivot {

class point2d : boost::multipliable<point2d, int32_t> {

public:
  point2d();

  point2d(std::array<int32_t, 2> coords);

  point2d(__m128i data) : data_(data) {}

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

  box2d(__m128i data) : data_(data) {}

  box2d(const std::array<interval2d, 2> &intervals);

  box2d(std::span<const point2d> points);

  __m128i data() const { return data_; }

  bool operator==(const box2d &b) const;

  bool operator!=(const box2d &b) const;

  interval2d operator[](int i) const;

  bool empty() const;

  box2d &operator+=(const point2d &b);

  box2d &operator-=(const point2d &b);

  box2d operator|(const box2d &b) const;

  box2d operator&(const box2d &b) const;

  std::string to_string() const;
};

class transform2d {

public:
  transform2d();

  transform2d(const std::array<int, 2> &perm, const std::array<int, 2> &signs);

  transform2d(const point2d &p, const point2d &q);

  template <typename Gen> static transform2d rand(Gen &gen) {
    static std::bernoulli_distribution flip_;

    std::array<int, 2> perm;
    std::array<int, 2> signs;
    for (int i = 0; i < 2; ++i) {
      perm[i] = i;
      signs[i] = 2 * flip_(gen) - 1;
    }
    std::shuffle(perm.begin(), perm.end(), gen);
    return transform2d(perm, signs);
  }

  static transform2d rand();

  bool operator==(const transform2d &t) const;

  point2d operator*(const point2d &p) const;

  transform2d operator*(const transform2d &t) const;

  box2d operator*(const box2d &b) const;

  bool is_identity() const;

  transform2d inverse() const;

  std::array<std::array<int, 2>, 2> to_matrix() const;

  std::string to_string() const;

private:
  __m128i perm_;
  __m128i signs_;
};

} // namespace pivot
