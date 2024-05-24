#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <span>
#include <vector>

#include <string>

namespace pivot {

enum angle { zero, ninety, one_eighty, two_seventy };

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

class rot {

public:
  rot();

  rot(angle a);

  rot(point<2> p, point<2> q);

  static rot rand();

  point<2> operator*(const point<2> &p) const;

  rot operator*(const rot &r) const;

  box<2> operator*(const box<2> &b) const;

  rot inverse() const;

  std::string to_string() const;

private:
  int cos_;
  int sin_;

  rot(int cos, int sin);
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
