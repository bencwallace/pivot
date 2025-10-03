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

static inline __m128i permutevar_epi32(__m128i data, __m128i perm) {
  return _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(data), perm));
}

static inline __m128i sort_bounds(__m128i pairs) {
  __m128i swapped = _mm_shuffle_epi32(pairs, _MM_SHUFFLE(1, 0, 3, 2));
  __m128i minima = _mm_min_epi32(pairs, swapped);
  __m128i maxima = _mm_max_epi32(pairs, swapped);
  return _mm_blend_epi32(minima, maxima, 0b1100);
}

namespace pivot {

// ============================================================================
// point<2> specialization
// ============================================================================

template <> class point<2> : boost::multipliable<point<2>, int> {
public:
  point();
  point(__m128i coords);
  point(const std::array<int, 2> &coords);

  static point unit(int i);

  int operator[](int i) const;
  bool operator==(const point &p) const;
  bool operator!=(const point &p) const;
  point operator+(const point &p) const;
  point operator-(const point &p) const;
  point &operator*=(int k);

  __m128i coords() const;
  int norm() const;
  std::string to_string() const;

private:
  __m128i coords_;
};

// Out-of-line definitions for point<2>
point<2>::point() : coords_(_mm_setzero_si128()) {}

point<2>::point(__m128i coords) : coords_(coords) {}

point<2>::point(const std::array<int, 2> &coords) : coords_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

point<2> point<2>::unit(int i) {
  __m128i coords = _mm_setzero_si128();
  coords = insert_epi32(coords, 1, i);
  return point(coords);
}

int point<2>::operator[](int i) const { return extract_epi32(coords_, i); }

bool point<2>::operator==(const point &p) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(coords_, p.coords_)) == 0xFFFF;
}

bool point<2>::operator!=(const point &p) const { return !(*this == p); }

point<2> point<2>::operator+(const point &p) const { return _mm_add_epi32(coords_, p.coords_); }

point<2> point<2>::operator-(const point &p) const { return _mm_sub_epi32(coords_, p.coords_); }

point<2> &point<2>::operator*=(int k) {
  coords_ = _mm_mullo_epi32(coords_, _mm_set1_epi32(k));
  return *this;
}

__m128i point<2>::coords() const { return coords_; }

int point<2>::norm() const {
  __m128i squared = _mm_mullo_epi32(coords_, coords_);
  return extract_epi32(_mm_hadd_epi32(squared, squared), 0);
}

std::string point<2>::to_string() const {
  std::string s = "(";
  s += std::to_string(extract_epi32(coords_, 0)) + ", ";
  s += std::to_string(extract_epi32(coords_, 1));
  s += ")";
  return s;
}

// ============================================================================
// box<2> specialization
// ============================================================================

template <> struct box<2> : boost::additive<box<2>, point<2>> {
  box() = delete;
  box(__m128i data);
  box(const std::array<interval, 2> &intervals);
  box(std::span<const point<2>> points);

  bool operator==(const box &b) const;
  bool operator!=(const box &b) const;
  interval operator[](int i) const;
  __m128i data() const;
  bool empty() const;

  box<2> &operator+=(const point<2> &b);
  box<2> &operator-=(const point<2> &b);
  box operator|(const box &b) const;
  box operator&(const box &b) const;

  std::string to_string() const;

private:
  __m128i data_; // x_min, y_min, x_max, y_max
};

// Out-of-line definitions for box<2>
box<2>::box(__m128i data) : data_(data) {}

box<2>::box(const std::array<interval, 2> &intervals)
    : data_(_mm_setr_epi32(intervals[0].left_, intervals[1].left_, intervals[0].right_, intervals[1].right_)) {}

box<2>::box(std::span<const point<2>> points) {
  std::array<int, 2> min;
  std::array<int, 2> max;
  min.fill(std::numeric_limits<int>::max());
  max.fill(std::numeric_limits<int>::min());
  for (const auto &p : points) {
    for (int i = 0; i < 2; ++i) {
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

bool box<2>::operator==(const box &b) const { return _mm_movemask_epi8(_mm_cmpeq_epi32(data_, b.data_)) == 0xFFFF; }

bool box<2>::operator!=(const box &b) const { return !(*this == b); }

interval box<2>::operator[](int i) const {
  return {int32_t(extract_epi32(data_, i)), int32_t(extract_epi32(data_, i + 2))};
}

__m128i box<2>::data() const { return data_; }

bool box<2>::empty() const {
  __m128i swapped = _mm_shuffle_epi32(data_, _MM_SHUFFLE(1, 0, 3, 2));
  __m128i cmp = _mm_cmpgt_epi32(data_, swapped);
  auto result = _mm_cvtsi128_si64(cmp);
  return result != 0;
}

box<2> &box<2>::operator+=(const point<2> &b) {
  __m128i offset = _mm_shuffle_epi32(b.coords(), _MM_SHUFFLE(1, 0, 1, 0));
  data_ = _mm_add_epi32(data_, offset);
  return *this;
}

box<2> &box<2>::operator-=(const point<2> &b) {
  __m128i offset = _mm_shuffle_epi32(b.coords(), _MM_SHUFFLE(1, 0, 1, 0));
  data_ = _mm_sub_epi32(data_, offset);
  return *this;
}

box<2> box<2>::operator|(const box &b) const {
  __m128i mins = _mm_min_epi32(data_, b.data_);
  __m128i maxs = _mm_max_epi32(data_, b.data_);
  return _mm_blend_epi32(mins, maxs, 0b1100);
}

box<2> box<2>::operator&(const box &b) const {
  __m128i mins = _mm_min_epi32(data_, b.data_);
  __m128i maxs = _mm_max_epi32(data_, b.data_);
  return _mm_blend_epi32(mins, maxs, 0b0011);
}

std::string box<2>::to_string() const {
  std::string s;
  s += "[";
  s += std::to_string(extract_epi32(data_, 0));
  s += ", ";
  s += std::to_string(extract_epi32(data_, 2));
  s += "] x [";
  s += std::to_string(extract_epi32(data_, 1));
  s += ", ";
  s += std::to_string(extract_epi32(data_, 3));
  s += "]";
  return s;
}

// ============================================================================
// transform<2> specialization
// ============================================================================

template <> class transform<2> {
public:
  transform();
  transform(__m128i perm, __m128i signs);
  transform(const std::array<int, 2> &perm, const std::array<int, 2> &signs);
  transform(const point<2> &p, const point<2> &q);

  template <typename Gen> static transform rand(Gen &gen);
  static transform rand();

  bool operator==(const transform &t) const;
  point<2> operator*(const point<2> &p) const;
  transform operator*(const transform &t) const;
  box<2> operator*(const box<2> &b) const;

  bool is_identity() const;
  transform inverse() const;
  std::array<std::array<int, 2>, 2> to_matrix() const;
  std::string to_string() const;

private:
  __m128i perm_;
  __m128i signs_;
};

// Out-of-line definitions for transform<2>
transform<2>::transform() : perm_(_mm_setr_epi32(0, 1, 2, 3)), signs_(_mm_set1_epi32(1)) {}

transform<2>::transform(__m128i perm, __m128i signs) : perm_(perm), signs_(signs) {}

transform<2>::transform(const std::array<int, 2> &perm, const std::array<int, 2> &signs)
    : perm_(_mm_setr_epi32(perm[0], perm[1], 2 + perm[0], 2 + perm[1])),
      signs_(_mm_setr_epi32(signs[0], signs[1], signs[0], signs[1])) {}

transform<2>::transform(const point<2> &p, const point<2> &q) : transform() {
  point<2> diff = q - p;
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

  perm_ = insert_epi32(perm_, idx, 0);
  perm_ = insert_epi32(perm_, 0, idx);
  signs_ = insert_epi32(signs_, -diff[idx], 0);
  signs_ = insert_epi32(signs_, diff[idx], idx);
}

template <typename Gen> transform<2> transform<2>::rand(Gen &gen) {
  static std::bernoulli_distribution flip_;

  std::array<int, 2> perm = {0, 1};
  std::array<int, 2> signs;
  for (int i = 0; i < 2; ++i) {
    signs[i] = 2 * flip_(gen) - 1;
  }
  std::shuffle(perm.begin(), perm.end(), gen);
  return transform(perm, signs);
}

transform<2> transform<2>::rand() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(gen);
}

bool transform<2>::operator==(const transform &t) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, t.signs_)) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, t.perm_)) == 0xFFFF;
}

point<2> transform<2>::operator*(const point<2> &p) const {
  return _mm_sign_epi32(permutevar_epi32(p.coords(), perm_), signs_);
}

transform<2> transform<2>::operator*(const transform &t) const {
  __m128i perm = permutevar_epi32(perm_, t.perm_);
  __m128i signs = _mm_sign_epi32(permutevar_epi32(t.signs_, perm_), signs_);
  return transform(perm, signs);
}

box<2> transform<2>::operator*(const box<2> &b) const {
  __m128i pairs = _mm_sign_epi32(permutevar_epi32(b.data(), perm_), signs_);
  return sort_bounds(pairs);
}

bool transform<2>::is_identity() const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, _mm_set1_epi32(1))) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, _mm_setr_epi32(0, 1, 2, 3))) == 0xFFFF;
}

transform<2> transform<2>::inverse() const { return transform(perm_, permutevar_epi32(signs_, perm_)); }

std::array<std::array<int, 2>, 2> transform<2>::to_matrix() const {
  std::array<std::array<int, 2>, 2> mat{};
  int p0 = extract_epi32(perm_, 0);
  int p1 = extract_epi32(perm_, 1);
  mat[p0][0] = extract_epi32(signs_, p0);
  mat[p1][1] = extract_epi32(signs_, p1);
  return mat;
}

std::string transform<2>::to_string() const {
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
  s += "]";
  return s;
}

// Point hash specialization for point<2>
template <> std::size_t point_hash::operator()<2>(const point<2> &p) const {
  std::size_t hash = 0;
  for (int i = 0; i < 2; ++i) {
    hash = num_steps_ + p[i] + 2 * num_steps_ * hash;
  }
  return hash;
}

// Explicit instantiations
template class point<2>;
template struct box<2>;
template class transform<2>;

} // namespace pivot
