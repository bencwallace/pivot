#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

template <int Dim> transform<Dim>::transform() {
  for (int i = 0; i < Dim; ++i) {
    perm_[i] = i;  // trivial permutation
    signs_[i] = 1; // standard orientations (no flips)
  }
}

template <int Dim>
transform<Dim>::transform(const std::array<int, Dim> &perm, const std::array<int, Dim> &signs)
    : perm_(perm), signs_(signs) {}

template <int Dim> transform<Dim>::transform(const point &p, const point &q) : transform() {
  // The input points should differ by 1 in a single coordinate. Start by finding this coordinate or fail.
  point diff = q - p;
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

template <int Dim> transform<Dim> transform<Dim>::rand() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(gen);
}

template <int Dim> bool transform<Dim>::operator==(const transform &t) const {
  return perm_ == t.perm_ && signs_ == t.signs_;
}

template <int Dim> point transform<Dim>::operator*(const point &p) const {
  std::vector<int> coords(Dim);
  for (int i = 0; i < Dim; ++i) {
    coords[perm_[i]] = signs_[perm_[i]] * p[i];
  }
  return point(coords);
}

template <int Dim> transform<Dim> transform<Dim>::operator*(const transform<Dim> &t) const {
  std::array<int, Dim> perm;
  std::array<int, Dim> signs;
  for (int i = 0; i < Dim; ++i) {
    perm[i] = perm_[t.perm_[i]];
    signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
  }
  return transform(perm, signs);
}

template <int Dim> box transform<Dim>::operator*(const box &b) const {
  std::vector<interval> intervals(Dim);
  for (int i = 0; i < Dim; ++i) {
    int x = signs_[perm_[i]] * b.intervals_[i].left_;
    int y = signs_[perm_[i]] * b.intervals_[i].right_;
    intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
  }
  return box(intervals);
}

template <int Dim> transform<Dim> transform<Dim>::inverse() const {
  std::array<int, Dim> perm;
  std::array<int, Dim> signs;
  for (int i = 0; i < Dim; ++i) {
    perm[perm_[i]] = i;
    signs[i] = signs_[perm_[i]];
  }
  return transform(perm, signs);
}

template <int Dim> std::array<std::array<int, Dim>, Dim> transform<Dim>::to_matrix() const {
  std::array<std::array<int, Dim>, Dim> matrix = {};
  for (int i = 0; i < Dim; ++i) {
    matrix[perm_[i]][i] = signs_[perm_[i]];
  }
  return matrix;
}

template <int Dim> std::string transform<Dim>::to_string() const {
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
