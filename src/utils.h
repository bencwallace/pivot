#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <span>
#include <vector>

#include <string>

namespace pivot {

template <int Dim> struct box;

struct interval {
  int left_;
  int right_;

  interval();
  interval(int left, int right);

  bool empty() const;

  std::string to_string() const;
};

template <int Dim> class point {

public:
  point() = default;

  point(std::array<int, Dim> coords) : coords_(coords) {}

  static point unit(int i) {
    point<Dim> result;
    result.coords_[i] = 1;
    return result;
  }

  int operator[](int i) const { return coords_[i]; }

  bool operator==(const point &p) const { return coords_ == p.coords_; }

  bool operator!=(const point &p) const { return coords_ != p.coords_; }

  point operator+(const point &p) const {
    std::array<int, Dim> sum;
    for (int i = 0; i < Dim; ++i) {
      sum[i] = coords_[i] + p.coords_[i];
    }
    return point(sum);
  }

  box<Dim> operator+(const box<Dim> &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(coords_[i] + b.intervals_[i].left_, coords_[i] + b.intervals_[i].right_);
    }
    return box<Dim>(intervals);
  }

  point operator-(const point &p) const {
    std::array<int, Dim> diff;
    for (int i = 0; i < Dim; ++i) {
      diff[i] = coords_[i] - p.coords_[i];
    }
    return point(diff);
  }

  friend point operator*(int k, const point &p) {
    std::array<int, Dim> coords;
    for (int i = 0; i < Dim; ++i) {
      coords[i] = k * p.coords_[i];
    }
    return point(coords);
  }

  std::string to_string() const {
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

private:
  std::array<int, Dim> coords_{};
};

template <int Dim> struct box {
  std::array<interval, Dim> intervals_;

  box() = delete;

  box(std::array<interval, Dim> intervals) : intervals_(intervals) {}

  box(std::span<const point<Dim>> points) {
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

  interval operator[](int i) const { return intervals_[i]; }

  bool empty() const {
    return std::any_of(intervals_.begin(), intervals_.end(), [](const interval &i) { return i.empty(); });
  }

  // union
  box operator+(const box &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(std::min(intervals_[i].left_, b.intervals_[i].left_),
                              std::max(intervals_[i].right_, b.intervals_[i].right_));
    }
    return box(intervals);
  }

  // intersection
  box operator*(const box &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(std::max(intervals_[i].left_, b.intervals_[i].left_),
                              std::min(intervals_[i].right_, b.intervals_[i].right_));
    }
    return box(intervals);
  }

  std::string to_string() const {
    std::string s = "";
    for (int i = 0; i < Dim - 1; ++i) {
      s += intervals_[i].to_string() + " x ";
    }
    s += intervals_[Dim - 1].to_string();
    return s;
  }
};

struct point_hash {
  int num_steps_;

  point_hash(int num_steps);

  template <int Dim> std::size_t operator()(const point<Dim> &p) const {
    std::size_t hash = 0;
    for (int i = 0; i < Dim; ++i) {
      hash = num_steps_ + p[i] + 2 * num_steps_ * hash;
    }
    return hash;
  }
};

template <int Dim> class transform {

public:
  transform() {
    for (int i = 0; i < Dim; ++i) {
      perm_[i] = i;
      signs_[i] = 1;
    }
  }

  transform(std::array<int, Dim> perm, std::array<int, Dim> signs) : perm_(perm), signs_(signs) {}

  transform(const point<Dim> &p, const point<Dim> &q) : transform() {
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
    perm_[0] = idx;
    perm_[idx] = 0;
    signs_[0] = -diff[idx];
    signs_[idx] = diff[idx];
  }

  static transform rand() {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[i] = i;
      signs[i] = 2 * (std::rand() % 2) - 1;
    }
    std::random_shuffle(perm.begin(), perm.end()); // NOLINT(clang-diagnostic-deprecated-declarations)
    return transform(perm, signs);
  }

  bool operator==(const transform &t) const { return perm_ == t.perm_ && signs_ == t.signs_; }

  point<Dim> operator*(const point<Dim> &p) const {
    std::array<int, Dim> coords;
    for (int i = 0; i < Dim; ++i) {
      coords[i] = signs_[i] * p[perm_[i]];
    }
    return point<Dim>(coords);
  }

  transform operator*(const transform &t) const {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[i] = perm_[t.perm_[i]];
      signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
    }
    return transform(perm, signs);
  }

  box<Dim> operator*(const box<Dim> &b) const {
    // TODO: test this
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      int x = signs_[perm_[i]] * b.intervals_[i].left_;
      int y = signs_[perm_[i]] * b.intervals_[i].right_;
      intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
    }
    return box<Dim>(intervals);
  }

  transform inverse() const {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[perm_[i]] = i;
      signs[i] = signs_[perm_[i]];
    }
    return transform(perm, signs);
  }

  std::array<std::array<int, Dim>, Dim> to_matrix() const {
    std::array<std::array<int, Dim>, Dim> matrix = {};
    for (int i = 0; i < Dim; ++i) {
      matrix[perm_[i]][i] = signs_[perm_[i]];
    }
    return matrix;
  }

  std::string to_string() const {
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

private:
  std::array<int, Dim> perm_;
  std::array<int, Dim> signs_;
};

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

} // namespace pivot
