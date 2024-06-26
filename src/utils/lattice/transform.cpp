#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

transform::transform(int dim) : dim_(dim) {
  perm_.reserve(dim);
  signs_.reserve(dim);
  for (int i = 0; i < dim; ++i) {
    perm_.push_back(i);  // trivial permutation
    signs_.push_back(1); // standard orientations (no flips)
  }
}

transform::transform(std::vector<int> &&perm, std::vector<int> &&signs)
    : dim_(perm.size()), perm_(std::move(perm)), signs_(std::move(signs)) {}

transform::transform(const point &p, const point &q) : transform(p.dim()) {
  // The input points should differ by 1 in a single coordinate. Start by finding this coordinate or fail.
  point diff = q - p;
  int idx = -1;
  for (int i = 0; i < dim_; ++i) {
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

transform transform::rand(int dim) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(dim, gen);
}

bool transform::operator==(const transform &t) const { return perm_ == t.perm_ && signs_ == t.signs_; }

point transform::operator*(const point &p) const {
  std::vector<int> coords(dim_);
  for (int i = 0; i < dim_; ++i) {
    coords[perm_[i]] = signs_[perm_[i]] * p[i];
  }
  return point(std::move(coords));
}

transform transform::operator*(const transform &t) const {
  std::vector<int> perm;
  std::vector<int> signs(dim_);
  perm.reserve(dim_);
  for (int i = 0; i < dim_; ++i) {
    perm.push_back(perm_[t.perm_[i]]);
    signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
  }
  return transform(std::move(perm), std::move(signs));
}

box transform::operator*(const box &b) const {
  std::vector<interval> intervals(dim_);
  for (int i = 0; i < dim_; ++i) {
    int x = signs_[perm_[i]] * b.intervals_[i].left_;
    int y = signs_[perm_[i]] * b.intervals_[i].right_;
    intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
  }
  return box(std::move(intervals));
}

transform transform::inverse() const {
  std::vector<int> perm(dim_);
  std::vector<int> signs;
  signs.reserve(dim_);
  for (int i = 0; i < dim_; ++i) {
    perm[perm_[i]] = i;
    signs.push_back(signs_[perm_[i]]);
  }
  return transform(std::move(perm), std::move(signs));
}

std::vector<std::vector<int>> transform::to_matrix() const {
  std::vector<std::vector<int>> matrix(dim_, std::vector<int>(dim_));
  for (int i = 0; i < dim_; ++i) {
    matrix[perm_[i]][i] = signs_[perm_[i]];
  }
  return matrix;
}

std::string transform::to_string() const {
  auto matrix = to_matrix();
  std::string s = "[";
  for (int i = 0; i < dim_; ++i) {
    s += "[";
    for (int j = 0; j < dim_ - 1; ++j) {
      s += std::to_string(matrix[i][j]) + ", ";
    }
    s += std::to_string(matrix[i][dim_ - 1]) + "]";
    if (i < dim_ - 1) {
      s += ", ";
    }
  }
  s += "]";
  return s;
}

} // namespace pivot
