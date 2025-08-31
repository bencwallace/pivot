#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

template <int Dim, bool Simd> transform<Dim, Simd>::transform() {
  for (int i = 0; i < Dim; ++i) {
    perm_[i] = i;  // trivial permutation
    signs_[i] = 1; // standard orientations (no flips)
  }
}

template <int Dim, bool Simd>
transform<Dim, Simd>::transform(const std::array<int, Dim> &perm, const std::array<int, Dim> &signs)
    : perm_(perm), signs_(signs) {}

template <int Dim, bool Simd>
transform<Dim, Simd>::transform(const point<Dim, Simd> &p, const point<Dim, Simd> &q) : transform() {
  // The input points should differ by 1 in a single coordinate. Start by finding this coordinate or fail.
  point<Dim, Simd> diff = q - p;
  int idx = -1;
  for (int i = 0; i < Dim; ++i) {
    if (std::abs(diff[i]) == 1) {
      if (idx == -1) {
        idx = i;
      } else { // a differing coordinate has already been found
        throw std::invalid_argument("Points are not adjacent");
      }
    }
  }
  if (idx == -1) {
    throw std::invalid_argument("Points are not adjacent");
  }

  // Construct the transform by modifying the identity transform.
  perm_[0] = idx;
  perm_[idx] = 0;
  signs_[0] = -diff[idx]; // not strictly necessary
  signs_[idx] = diff[idx];
}

template <int Dim, bool Simd> transform<Dim, Simd> transform<Dim, Simd>::rand() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(gen);
}

template <int Dim, bool Simd> bool transform<Dim, Simd>::operator==(const transform &t) const {
  return perm_ == t.perm_ && signs_ == t.signs_;
}

template <int Dim, bool Simd> point<Dim, Simd> transform<Dim, Simd>::operator*(const point<Dim, Simd> &p) const {
  std::array<int, Dim> coords;
  for (int i = 0; i < Dim; ++i) {
    coords[perm_[i]] = signs_[perm_[i]] * p[i];
  }
  return point<Dim, Simd>(coords);
}

template <int Dim, bool Simd>
transform<Dim, Simd> transform<Dim, Simd>::operator*(const transform<Dim, Simd> &t) const {
  std::array<int, Dim> perm;
  std::array<int, Dim> signs;
  for (int i = 0; i < Dim; ++i) {
    perm[i] = perm_[t.perm_[i]];
    signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
  }
  return transform(perm, signs);
}

template <int Dim, bool Simd> box<Dim, Simd> transform<Dim, Simd>::operator*(const box<Dim, Simd> &b) const {
  std::array<interval, Dim> intervals;
  for (int i = 0; i < Dim; ++i) {
    int x = signs_[perm_[i]] * b.intervals_[i].left_;
    int y = signs_[perm_[i]] * b.intervals_[i].right_;
    intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
  }
  return box<Dim, Simd>(intervals);
}

template <int Dim, bool Simd> bool transform<Dim, Simd>::is_identity() const {
  for (int i = 0; i < Dim; ++i) {
    if (perm_[i] != i || signs_[i] != 1) {
      return false;
    }
  }
  return true;
}

template <int Dim, bool Simd> transform<Dim, Simd> transform<Dim, Simd>::inverse() const {
  std::array<int, Dim> perm;
  std::array<int, Dim> signs;
  for (int i = 0; i < Dim; ++i) {
    perm[perm_[i]] = i;
    signs[i] = signs_[perm_[i]];
  }
  return transform(perm, signs);
}

template <int Dim, bool Simd> std::array<std::array<int, Dim>, Dim> transform<Dim, Simd>::to_matrix() const {
  std::array<std::array<int, Dim>, Dim> matrix = {};
  for (int i = 0; i < Dim; ++i) {
    matrix[perm_[i]][i] = signs_[perm_[i]];
  }
  return matrix;
}

template <int Dim, bool Simd> std::string transform<Dim, Simd>::to_string() const {
  auto matrix = to_matrix();
  std::string s = "[";
  for (int i = 0; i < Dim; ++i) {
    s += "[";
    for (int j = 0; j < Dim - 1; ++j) {
      s += std::to_string(matrix[i][j]) + ", ";
    }
    s += std::to_string(matrix[i][Dim - 1]) + "]";
    if (i < Dim - 1) {
      s += ", ";
    }
  }
  s += "]";
  return s;
}

} // namespace pivot
