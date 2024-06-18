#include <fstream>
#include <random>
#include <stdexcept>
#include <vector>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "defines.h"
#include "lattice.h"

namespace pivot {

/* point and point_hash */

template <int Dim> point<Dim>::point(const std::array<int, Dim> &coords) : coords_(coords) {}

template <int Dim> point<Dim> point<Dim>::unit(int i) {
  point<Dim> result;
  result.coords_[i] = 1;
  return result;
}

template <int Dim> int point<Dim>::operator[](int i) const { return coords_[i]; }

template <int Dim> bool point<Dim>::operator==(const point &p) const { return coords_ == p.coords_; }

template <int Dim> bool point<Dim>::operator!=(const point &p) const { return coords_ != p.coords_; }

template <int Dim> point<Dim> point<Dim>::operator+(const point<Dim> &p) const {
  std::array<int, Dim> sum;
  for (int i = 0; i < Dim; ++i) {
    sum[i] = coords_[i] + p.coords_[i];
  }
  return point(sum);
}

template <int Dim> box<Dim> &box<Dim>::operator+=(const point<Dim> &p) {
  for (int i = 0; i < Dim; ++i) {
    intervals_[i].left_ += p[i];
    intervals_[i].right_ += p[i];
  }
  return *this;
}

template <int Dim> box<Dim> &box<Dim>::operator-=(const point<Dim> &b) {
  for (int i = 0; i < Dim; ++i) {
    intervals_[i].left_ -= b[i];
    intervals_[i].right_ -= b[i];
  }
  return *this;
}

template <int Dim> point<Dim> point<Dim>::operator-(const point &p) const {
  std::array<int, Dim> diff;
  for (int i = 0; i < Dim; ++i) {
    diff[i] = coords_[i] - p.coords_[i];
  }
  return point(diff);
}

template <int Dim> point<Dim> &point<Dim>::operator*=(int k) {
  for (int i = 0; i < Dim; ++i) {
    coords_[i] *= k;
  }
  return *this;
}

template <int Dim> int point<Dim>::norm() const {
  int sum = 0;
  for (int i = 0; i < Dim; ++i) {
    sum += coords_[i] * coords_[i];
  }
  return sum;
}

template <int Dim> std::string point<Dim>::to_string() const {
  std::string s = "(";
  for (int i = 0; i < Dim - 1; ++i) {
    s += std::to_string(coords_[i]) + ", ";
  }
  s += std::to_string(coords_[Dim - 1]);
  if constexpr (Dim == 1) {
    s += ",)";
  } else {
    s += ")";
  }
  return s;
}

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

template <int Dim> std::size_t point_hash::operator()(const point<Dim> &p) const {
  std::size_t hash = 0;
  for (int i = 0; i < Dim; ++i) {
    hash = num_steps_ + p[i] + 2 * num_steps_ * hash;
  }
  return hash;
}

/* interval and box */

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

// TODO: don't distinguish empty intervals
bool interval::operator==(const interval &other) const { return left_ == other.left_ && right_ == other.right_; }

bool interval::operator!=(const interval &other) const { return !(*this == other); }

bool interval::empty() const { return left_ > right_; }

/** @brief Returns the string of the form "[{left_}, {right_}]". */
std::string interval::to_string() const {
  // for some reason, gcc-12 gives -Wrestrict warning if I use the string concatenation operator
  std::string result = "[";
  result.append(std::to_string(left_)).append(", ").append(std::to_string(right_)).append("]");
  return result;
}

template <int Dim> box<Dim>::box(const std::array<interval, Dim> &intervals) : intervals_(intervals) {}

template <int Dim> box<Dim>::box(std::span<const point<Dim>> points) {
  std::array<int, Dim> min;
  std::array<int, Dim> max;
  min.fill(std::numeric_limits<int>::max());
  max.fill(std::numeric_limits<int>::min());
  for (const auto &p : points) {
    for (int i = 0; i < Dim; ++i) {
      min[i] = std::min(min[i], p[i]);
      max[i] = std::max(max[i], p[i]);
    }
  }
  // anchor at (1, 0, ..., 0)
  intervals_[0] = interval(min[0] - points[0][0] + 1, max[0] - points[0][0] + 1);
  for (int i = 1; i < Dim; ++i) {
    intervals_[i] = interval(min[i] - points[0][i], max[i] - points[0][i]);
  }
}

template <int Dim> bool box<Dim>::operator==(const box &b) const { return intervals_ == b.intervals_; }

template <int Dim> bool box<Dim>::operator!=(const box &b) const { return intervals_ != b.intervals_; }

template <int Dim> interval box<Dim>::operator[](int i) const { return intervals_[i]; }

template <int Dim> bool box<Dim>::empty() const {
  return std::any_of(intervals_.begin(), intervals_.end(), [](const interval &i) { return i.empty(); });
}

template <int Dim> box<Dim> box<Dim>::operator|(const box<Dim> &b) const {
  std::array<interval, Dim> intervals;
  for (int i = 0; i < Dim; ++i) {
    intervals[i] = interval(std::min(intervals_[i].left_, b.intervals_[i].left_),
                            std::max(intervals_[i].right_, b.intervals_[i].right_));
  }
  return box(intervals);
}

template <int Dim> box<Dim> box<Dim>::operator&(const box<Dim> &b) const {
  std::array<interval, Dim> intervals;
  for (int i = 0; i < Dim; ++i) {
    intervals[i] = interval(std::max(intervals_[i].left_, b.intervals_[i].left_),
                            std::min(intervals_[i].right_, b.intervals_[i].right_));
  }
  return box(intervals);
}

template <int Dim> std::string box<Dim>::to_string() const {
  std::string s = "";
  for (int i = 0; i < Dim - 1; ++i) {
    s += intervals_[i].to_string() + " x ";
  }
  s += intervals_[Dim - 1].to_string();
  return s;
}

/* transform */

template <int Dim> transform<Dim>::transform() {
  for (int i = 0; i < Dim; ++i) {
    perm_[i] = i;  // trivial permutation
    signs_[i] = 1; // standard orientations (no flips)
  }
}

template <int Dim>
transform<Dim>::transform(const std::array<int, Dim> &perm, const std::array<int, Dim> &signs)
    : perm_(perm), signs_(signs) {}

template <int Dim> transform<Dim>::transform(const point<Dim> &p, const point<Dim> &q) : transform() {
  // The input points should differ by 1 in a single coordinate. Start by finding this coordinate or fail.
  point<Dim> diff = q - p;
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

template <int Dim> point<Dim> transform<Dim>::operator*(const point<Dim> &p) const {
  std::array<int, Dim> coords;
  for (int i = 0; i < Dim; ++i) {
    coords[i] = signs_[i] * p[perm_[i]];
  }
  return point<Dim>(coords);
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

template <int Dim> box<Dim> transform<Dim>::operator*(const box<Dim> &b) const {
  std::array<interval, Dim> intervals;
  for (int i = 0; i < Dim; ++i) {
    int x = signs_[perm_[i]] * b.intervals_[i].left_;
    int y = signs_[perm_[i]] * b.intervals_[i].right_;
    intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
  }
  return box<Dim>(intervals);
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

/* misc */

template <int Dim> std::vector<point<Dim>> from_csv(const std::string &path) {
  std::ifstream file(path);
  std::vector<point<Dim>> points;
  std::string line;
  while (std::getline(file, line)) {
    std::array<int, Dim> coords;
    size_t start = 0;
    for (int i = 0; i < Dim; ++i) {
      size_t end = line.find(',', start);
      if (end == std::string::npos && i < Dim - 1) {
        throw std::invalid_argument("Invalid CSV format at line " + std::to_string(points.size()));
      }
      coords[i] = std::stoi(line.substr(start, end - start));
      start = end + 1;
    }
    points.push_back(point<Dim>(coords));
  }
  return points;
}

template <int Dim> void to_csv(const std::string &path, const std::vector<point<Dim>> &points) {
  // TODO: check path exists
  std::ofstream file(path);
  for (const auto &p : points) {
    for (int i = 0; i < Dim - 1; ++i) {
      file << p[i] << ",";
    }
    file << p[Dim - 1] << std::endl;
  }
}

template <int Dim> std::vector<point<Dim>> line(int num_steps) {
  std::vector<point<Dim>> steps(num_steps);
  for (int i = 0; i < num_steps; ++i) {
    steps[i] = (i + 1) * point<Dim>::unit(0);
  }
  return steps;
}

/* explicit instantiation */

#define POINT_INST(z, n, data) template class point<n>;
#define BOX_INST(z, n, data) template struct box<n>;
#define TRANSFORM_INST(z, n, data) template class transform<n>;
#define POINT_HASH_CALL_INST(z, n, data) template std::size_t point_hash::operator()<n>(const point<n> &p) const;
#define FROM_CSV_INST(z, n, data) template std::vector<point<n>> from_csv<n>(const std::string &path);
#define TO_CSV_INST(z, n, data) template void to_csv<n>(const std::string &path, const std::vector<point<n>> &points);
#define LINE_INST(z, n, data) template std::vector<point<n>> line(int num_steps);

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, POINT_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, BOX_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TRANSFORM_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, POINT_HASH_CALL_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TO_CSV_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, FROM_CSV_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, LINE_INST, ~)

} // namespace pivot
