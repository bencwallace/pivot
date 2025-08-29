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

// Given a vector (a, b, c, d) representing potentially unsorted intervals
// {a, c} and {b, d}, return [min(a, c), min(b, d), max(a, c), max(b, d)]
// representing intervals [min(a, c), max(a, c)] and [min(b, d), max(b, d)].
inline __m128i sort_bounds(__m128i pairs) {
  __m128i swapped = _mm_shuffle_epi32(pairs, _MM_SHUFFLE(1, 0, 3, 2));
  __m128i minima = _mm_min_epi32(pairs, swapped);
  __m128i maxima = _mm_max_epi32(pairs, swapped);
  return _mm_blend_epi32(minima, maxima, 0b1100);
}

inline __m128i permutevar_epi32(__m128i data, __m128i perm) {
  return _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(data), perm));
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

transform2d::transform2d() : perm_(_mm_setr_epi32(0, 1, 2, 3)), signs_(_mm_set1_epi32(1)) {}

transform2d::transform2d(const std::array<int, 2> &perm, const std::array<int, 2> &signs)
    : perm_(_mm_setr_epi32(perm[0], perm[1], 2 + perm[0], 2 + perm[1])),
      signs_(_mm_setr_epi32(signs[0], signs[1], signs[0], signs[1])) {}

transform2d::transform2d(const point2d &p, const point2d &q) {
  point2d diff = q - p;
  int idx = -1;
  for (int i = 0; i < 2; ++i) {
    if (std::abs(diff[i]) == 1) {
      if (idx == -1) {
        idx = i;
      } else {
        throw std::invalid_argument("Points are not adjacent");
      }
    }
  }
  if (idx == -1) {
    throw std::invalid_argument("Points are not adjacent");
  }

  insert_epi32(perm_, idx, 0);
  insert_epi32(perm_, 0, idx);
  insert_epi32(signs_, -diff[idx], 0);
  insert_epi32(signs_, diff[idx], idx);
}

transform2d transform2d::rand() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(gen);
}

bool transform2d::operator==(const transform2d &t) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, t.signs_)) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, t.perm_)) == 0xFFFF;
}

point2d transform2d::operator*(const point2d &p) const {
  return _mm_sign_epi32(permutevar_epi32(p.data(), perm_), signs_);
}

transform2d transform2d::operator*(const transform2d &t) const {
  __m128i perm = permutevar_epi32(perm_, t.perm_);
  __m128i signs = _mm_sign_epi32(permutevar_epi32(t.signs_, perm_), signs_);
  return {perm, signs};
}

box2d transform2d::operator*(const box2d &b) const {
  __m128i pairs = _mm_sign_epi32(permutevar_epi32(b.data(), perm_), signs_);
  return sort_bounds(pairs);
}

bool transform2d::is_identity() const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, _mm_set1_epi32(1))) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, _mm_setr_epi32(0, 1, 2, 3))) == 0xFFFF;
}

transform2d transform2d::inverse() const {
  // In general, the inverse is given by signs S' and permutations P' such
  // that P' = P^-1 and S' = S P. The latter's components can be obtained by
  // viewing S as a vector and applying P to it (i.e. permuting it). Moreover,
  // in 2D, P^-1 is the same as P.
  return transform2d(perm_, permutevar_epi32(signs_, perm_));
}

std::array<std::array<int, 2>, 2> transform2d::to_matrix() const {
  std::array<std::array<int, 2>, 2> mat;
  mat[perm_[0]][0] = extract_epi32(signs_, extract_epi32(perm_, 0));
  mat[perm_[1]][1] = extract_epi32(signs_, extract_epi32(perm_, 1));
  return mat;
}

std::string transform2d::to_string() const {
  auto matrix = to_matrix();
  std::string s = "[";
  for (int i = 0; i < 2; ++i) {
    s += "[";
    s += std::to_string(matrix[i][0]) + ", ";
    s += std::to_string(matrix[i][1]) + "]";
    if (i < 1) {
      s += ", ";
    }
  }
  return s;
}

} // namespace pivot
