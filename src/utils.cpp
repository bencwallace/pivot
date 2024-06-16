#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

#include <boost/unordered_map.hpp>

#include "utils.h"

namespace pivot {

/* point and point_hash */

point::point(int dim) : coords_(dim) {}

point::point(std::vector<int> &&coords) : coords_(std::move(coords)) {}

point::point(const point &p) : coords_(p.coords_) {}

point &point::operator=(const point &p) {
  coords_ = p.coords_;
  return *this;
}

point point::unit(int dim, int i) {
  std::vector<int> coords(dim, 0);
  coords[i] = 1;
  return point(std::move(coords));
}

int point::dim() const { return coords_.size(); }

int point::operator[](int i) const { return coords_[i]; }

bool point::operator==(const point &p) const { return coords_ == p.coords_; }

bool point::operator!=(const point &p) const { return coords_ != p.coords_; }

point point::operator+(const point &p) const {
  std::vector<int> sum;
  sum.reserve(dim());
  for (int i = 0; i < dim(); ++i) {
    sum.push_back(coords_[i] + p[i]);
  }
  return point(std::move(sum));
}

point &point::operator+=(const point &p) {
  for (int i = 0; i < dim(); ++i) {
    coords_[i] += p[i];
  }
  return *this;
}

box point::operator+(const box &b) const {
  std::vector<interval> intervals;
  intervals.reserve(dim());
  for (int i = 0; i < dim(); ++i) {
    intervals.push_back(interval(coords_[i] + b.intervals_[i].left_, coords_[i] + b.intervals_[i].right_));
  }
  return box(std::move(intervals));
}

point point::operator-(const point &p) const {
  std::vector<int> diff;
  diff.reserve(dim());
  for (int i = 0; i < dim(); ++i) {
    diff.push_back(coords_[i] - p[i]);
  }
  return point(std::move(diff));
}

point operator*(int k, const point &p) {
  std::vector<int> coords;
  coords.reserve(p.dim());
  for (int i = 0; i < p.dim(); ++i) {
    coords.push_back(k * p[i]);
  }
  return point(std::move(coords));
}

std::string point::to_string() const {
  std::string s = "(";
  for (int i = 0; i < dim(); ++i) {
    s += std::to_string(coords_[i]);
    if (i < dim() - 1) {
      s += ", ";
    }
  }
  s += ")";
  return s;
}

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

std::size_t point_hash::operator()(const point &p) const {
  std::size_t seed = 0;
  for (int i = 0; i < p.dim(); ++i) {
    boost::hash_combine(seed, p[i]);
  }
  return seed;
}

/* interval and box */

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

bool interval::empty() const { return left_ > right_; }

std::string interval::to_string() const { return "[" + std::to_string(left_) + ", " + std::to_string(right_) + "]"; }

box::box(int dim) : intervals_(dim) {}

box::box(std::vector<interval> &&intervals) : intervals_(std::move(intervals)) {}

box::box(const box &b) : intervals_(b.intervals_) {}

box &box::operator=(const box &b) {
  intervals_ = b.intervals_;
  return *this;
}

int box::dim() const { return intervals_.size(); }

box::box(std::span<const point> points) {
  int dim = points[0].dim();
  std::vector<int> min(dim);
  std::vector<int> max(dim);
  for (int i = 0; i < dim; ++i) {
    min[i] = std::numeric_limits<int>::max();
    max[i] = std::numeric_limits<int>::min();
  }
  for (const auto &p : points) {
    for (int i = 0; i < dim; ++i) {
      min[i] = std::min(min[i], p[i]);
      max[i] = std::max(max[i], p[i]);
    }
  }

  intervals_.resize(dim);
  // anchor at (1, 0, ..., 0)
  intervals_[0] = interval(min[0] - points[0][0] + 1, max[0] - points[0][0] + 1);
  for (int i = 1; i < dim; ++i) {

    intervals_[i] = interval(min[i] - points[0][i], max[i] - points[0][i]);
  }
}

bool box::empty() const {
  return std::any_of(intervals_.begin(), intervals_.end(), [](const interval &i) { return i.empty(); });
}

box box::operator+(const box &b1) const {
  std::vector<interval> intervals;
  intervals.reserve(dim());
  for (int i = 0; i < dim(); ++i) {
    intervals.push_back(interval(std::min(intervals_[i].left_, b1.intervals_[i].left_),
                                 std::max(intervals_[i].right_, b1.intervals_[i].right_)));
  }
  return box(std::move(intervals));
}

box &box::operator+=(const point &p) {
  for (int i = 0; i < dim(); ++i) {
    intervals_[i].left_ += p[i];
    intervals_[i].right_ += p[i];
  }
  return *this;
}

box &box::operator+=(const box &b) {
  for (int i = 0; i < dim(); ++i) {
    intervals_[i].left_ = std::min(intervals_[i].left_, b.intervals_[i].left_);
    intervals_[i].right_ = std::max(intervals_[i].right_, b.intervals_[i].right_);
  }
  return *this;
}

box box::operator*(const box &b1) const {
  std::vector<interval> intervals;
  intervals.reserve(dim());
  for (int i = 0; i < dim(); ++i) {
    intervals.push_back(interval(std::max(intervals_[i].left_, b1.intervals_[i].left_),
                                 std::min(intervals_[i].right_, b1.intervals_[i].right_)));
  }
  return box(std::move(intervals));
}

box &box::operator*=(const box &b) {
  for (int i = 0; i < dim(); ++i) {
    intervals_[i].left_ = std::max(intervals_[i].left_, b.intervals_[i].left_);
    intervals_[i].right_ = std::min(intervals_[i].right_, b.intervals_[i].right_);
  }
  return *this;
}

std::string box::to_string() const {
  std::string s = "";
  for (int i = 0; i < dim(); ++i) {
    s += intervals_[i].to_string();
    if (i < dim() - 1) {
      s += " x ";
    }
  }
  return s;
}

/* transform */

transform::transform(int dim) : perm_(), signs_(dim, 1) {
  for (int i = 0; i < dim; ++i) {
    perm_.push_back(i);
  }
}

transform::transform(std::vector<int> &&perm, std::vector<int> &&signs)
    : perm_(std::move(perm)), signs_(std::move(signs)) {}

transform::transform(const point &p, const point &q) : transform(p.dim()) {
  point diff = q - p;
  int idx = -1;
  for (int i = 0; i < dim(); ++i) {
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
  perm_[0] = idx;
  perm_[idx] = 0;
  signs_[0] = -diff[idx];
  signs_[idx] = diff[idx];
}

transform transform::rand(int dim) {
  std::vector<int> perm(dim);
  std::vector<int> signs(dim);
  for (int i = 0; i < dim; ++i) {
    perm[i] = i;
    signs[i] = 2 * (std::rand() % 2) - 1;
  }
  std::random_shuffle(perm.begin(), perm.end());
  return transform(std::move(perm), std::move(signs));
}

int transform::dim() const { return perm_.size(); }

bool transform::operator==(const transform &t) const { return perm_ == t.perm_ && signs_ == t.signs_; }

point transform::operator*(const point &p) const {
  std::vector<int> coords(dim());
  for (int i = 0; i < dim(); ++i) {
    coords[i] = signs_[i] * p[perm_[i]];
  }
  return point(std::move(coords));
}

point &point::operator*=(const transform &t) {
  static std::vector<int> temp(dim());
  for (int i = 0; i < dim(); ++i) {
    temp[i] = t.signs_[i] * coords_[t.perm_[i]];
  }
  coords_ = temp;
  return *this;
}

box transform::operator*(const box &b) const {
  // TODO: test this
  std::vector<interval> intervals(dim());
  for (int i = 0; i < dim(); ++i) {
    int x = signs_[perm_[i]] * b.intervals_[i].left_;
    int y = signs_[perm_[i]] * b.intervals_[i].right_;
    intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
  }
  return box(std::move(intervals));
}

box &box::operator*=(const transform &t) {
  static std::vector<interval> temp(dim());
  for (int i = 0; i < dim(); ++i) {
    int x = t.signs_[t.perm_[i]] * intervals_[i].left_;
    int y = t.signs_[t.perm_[i]] * intervals_[i].right_;
    temp[t.perm_[i]].left_ = std::min(x, y);
    temp[t.perm_[i]].right_ = std::max(x, y);
  }
  intervals_ = temp;
  return *this;
}

transform transform::operator*(const transform &t) const {
  std::vector<int> perm(dim());
  std::vector<int> signs(dim());
  for (int i = 0; i < dim(); ++i) {
    perm[i] = perm_[t.perm_[i]];
    signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
  }
  return transform(std::move(perm), std::move(signs));
}

transform transform::inverse() const {
  std::vector<int> perm(dim());
  std::vector<int> signs(dim());
  for (int i = 0; i < dim(); ++i) {
    perm[perm_[i]] = i;
    signs[i] = signs_[perm_[i]];
  }
  return transform(std::move(perm), std::move(signs));
}

std::vector<std::vector<int>> transform::to_matrix() const {
  std::vector<std::vector<int>> matrix(dim(), std::vector<int>(dim(), 0));
  for (int i = 0; i < dim(); ++i) {
    matrix[perm_[i]][i] = signs_[perm_[i]];
  }
  return matrix;
}

std::string transform::to_string() const {
  auto matrix = to_matrix();
  std::string s = "[";
  for (int i = 0; i < dim(); ++i) {
    s += "[";
    for (int j = 0; j < dim() - 1; ++j) {
      s += std::to_string(matrix[i][j]) + ", ";
    }
    s += std::to_string(matrix[i][dim() - 1]) + "]";
    if (i < dim() - 1) {
      s += ", ";
    }
  }
  s += "]";
  return s;
}

/* misc */

void to_csv(const std::string &path, const std::vector<point> &points) {
  // TODO: check path exists
  std::ofstream file(path);
  for (const auto &p : points) {
    for (int i = 0; i < p.dim(); ++i) {
      file << p[i];
      if (i < p.dim() - 1) {
        file << ",";
      }
    }
    file << std::endl;
  }
  file.close();
}

} // namespace pivot
