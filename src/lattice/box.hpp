#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

bool interval::operator==(const interval &other) const {
  return (left_ == other.left_ && right_ == other.right_) || (empty() && other.empty());
}

bool interval::operator!=(const interval &other) const { return !(*this == other); }

bool interval::empty() const { return left_ > right_; }

/** @brief Returns the string of the form "[{left_}, {right_}]". */
std::string interval::to_string() const {
  // for some reason, gcc-12 gives -Wrestrict warning if I use the string concatenation operator
  std::string result = "[";
  result.append(std::to_string(left_)).append(", ").append(std::to_string(right_)).append("]");
  return result;
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

} // namespace pivot
