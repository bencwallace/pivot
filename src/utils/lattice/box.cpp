#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

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

box &box::operator+=(const point &p) {
  for (int i = 0; i < dim_; ++i) {
    intervals_[i].left_ += p[i];
    intervals_[i].right_ += p[i];
  }
  return *this;
}

box &box::operator-=(const point &b) {
  for (int i = 0; i < dim_; ++i) {
    intervals_[i].left_ -= b[i];
    intervals_[i].right_ -= b[i];
  }
  return *this;
}

box::box(int dim) : intervals_(dim) {}

box::box(const std::vector<interval> &intervals) : dim_(intervals.size()), intervals_(intervals) {}

box::box(std::span<const point> points) : dim_(points[0].dim()) {
  std::vector<int> min(dim_, std::numeric_limits<int>::max());
  std::vector<int> max(dim_, std::numeric_limits<int>::min());
  for (const auto &p : points) {
    for (int i = 0; i < dim_; ++i) {
      min[i] = std::min(min[i], p[i]);
      max[i] = std::max(max[i], p[i]);
    }
  }

  intervals_.reserve(dim_);
  // anchor at (1, 0, ..., 0)
  intervals_.emplace_back(min[0] - points[0][0] + 1, max[0] - points[0][0] + 1);
  for (int i = 1; i < dim_; ++i) {
    intervals_.emplace_back(min[i] - points[0][i], max[i] - points[0][i]);
  }
}

bool box::operator==(const box &b) const { return intervals_ == b.intervals_; }

bool box::operator!=(const box &b) const { return intervals_ != b.intervals_; }

interval box::operator[](int i) const { return intervals_[i]; }

bool box::empty() const {
  return std::any_of(intervals_.begin(), intervals_.end(), [](const interval &i) { return i.empty(); });
}

box box::operator|(const box &b) const {
  std::vector<interval> intervals;
  intervals.reserve(dim_);
  for (int i = 0; i < dim_; ++i) {
    intervals.emplace_back(std::min(intervals_[i].left_, b.intervals_[i].left_),
                           std::max(intervals_[i].right_, b.intervals_[i].right_));
  }
  return box(intervals);
}

box box::operator&(const box &b) const {
  std::vector<interval> intervals;
  intervals.reserve(dim_);
  for (int i = 0; i < dim_; ++i) {
    intervals.push_back(interval(std::max(intervals_[i].left_, b.intervals_[i].left_),
                                 std::min(intervals_[i].right_, b.intervals_[i].right_)));
  }
  return box(intervals);
}

std::string box::to_string() const {
  std::string s = "";
  for (int i = 0; i < dim_ - 1; ++i) {
    s += intervals_[i].to_string() + " x ";
  }
  s += intervals_[dim_ - 1].to_string();
  return s;
}

} // namespace pivot
