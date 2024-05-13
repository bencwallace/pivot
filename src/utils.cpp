#include <algorithm>
#include <stdexcept>

#include "utils.h"

namespace pivot {

point::point() : x_(0), y_(0) {}

point::point(int x, int y) : x_(x), y_(y) {}

int point::x() const { return x_; }

int point::y() const { return y_; }

bool point::operator==(const point &p) const { return x_ == p.x() && y_ == p.y(); }

bool point::operator!=(const point &p) const { return x_ != p.x() || y_ != p.y(); }

point point::operator+(const point &p) const { return point(x_ + p.x(), y_ + p.y()); }

box point::operator+(const box &b) const {
  return box(interval(x_ + b.x_.left_, x_ + b.x_.right_), interval(y_ + b.y_.left_, y_ + b.y_.right_));
}

point point::operator-(const point &p) const { return point(x_ - p.x(), y_ - p.y()); }

std::string point::to_string() const { return "(" + std::to_string(x_) + ", " + std::to_string(y_) + ")"; }

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

std::size_t point_hash::operator()(const point &p) const { return p.x() + num_steps_ * p.y(); }

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

bool interval::empty() const { return left_ > right_; }

std::string interval::to_string() const { return "[" + std::to_string(left_) + ", " + std::to_string(right_) + "]"; }

box::box(interval x, interval y) : x_(x), y_(y) {}

box::box(int n, point *points) {
  int min_x = points[0].x();
  int max_x = points[0].x();
  int min_y = points[0].y();
  int max_y = points[0].y();
  for (int i = 1; i < n; ++i) {
    min_x = std::min(min_x, points[i].x());
    max_x = std::max(max_x, points[i].x());
    min_y = std::min(min_y, points[i].y());
    max_y = std::max(max_y, points[i].y());
  }

  // anchor at (1, 0)
  x_.left_ = min_x - points[0].x() + 1;
  x_.right_ = max_x - points[0].x() + 1;
  y_.left_ = min_y - points[0].y();
  y_.right_ = max_y - points[0].y();
}

bool box::empty() const { return x_.empty() || y_.empty(); }

box box::operator+(const box &b1) const {
  int min_x = std::min(x_.left_, b1.x_.left_);
  int max_x = std::max(x_.right_, b1.x_.right_);
  int min_y = std::min(y_.left_, b1.y_.left_);
  int max_y = std::max(y_.right_, b1.y_.right_);
  return box(interval(min_x, max_x), interval(min_y, max_y));
}

box box::operator*(const box &b1) const {
  int min_x = std::max(x_.left_, b1.x_.left_);
  int max_x = std::min(x_.right_, b1.x_.right_);
  int min_y = std::max(y_.left_, b1.y_.left_);
  int max_y = std::min(y_.right_, b1.y_.right_);
  return box(interval(min_x, max_x), interval(min_y, max_y));
}

std::string box::to_string() const { return x_.to_string() + " x " + y_.to_string(); }

rot::rot() : cos_(1), sin_(0) {}

rot::rot(angle a) {
  switch (a) {
  case zero:
    cos_ = 1;
    sin_ = 0;
    break;
  case ninety:
    cos_ = 0;
    sin_ = 1;
    break;
  case one_eighty:
    cos_ = -1;
    sin_ = 0;
    break;
  case two_seventy:
    cos_ = 0;
    sin_ = -1;
    break;
  }
}

rot::rot(point p, point q) {
  auto dx = q.x() - p.x();
  auto dy = q.y() - p.y();
  if (!((std::abs(dx) == 1) ^ (std::abs(dy) == 1))) {
    throw std::invalid_argument("Points are not adjacent");
  }
  if (dx == 0) {
    if (dy == 1) {
      cos_ = 0;
      sin_ = 1;
    } else if (dy == -1) {
      cos_ = 0;
      sin_ = -1;
    }
  } else if (dy == 0) {
    if (dx == 1) {
      cos_ = 1;
      sin_ = 0;
    } else if (dx == -1) {
      cos_ = -1;
      sin_ = 0;
    }
  }
}

rot::rot(int cos, int sin) : cos_(cos), sin_(sin) {}

rot rot::rand() {
  auto r = 1 + std::rand() % 3;
  return rot(static_cast<angle>(r));
}

point rot::operator*(const point &p) const { return point(cos_ * p.x() - sin_ * p.y(), sin_ * p.x() + cos_ * p.y()); }

rot rot::operator*(const rot &r) const { return rot(cos_ * r.cos_ - sin_ * r.sin_, sin_ * r.cos_ + cos_ * r.sin_); }

box rot::operator*(const box &b) const {
  auto p1 = point(b.x_.left_, b.y_.left_);
  auto p2 = point(b.x_.right_, b.y_.left_);
  auto p3 = point(b.x_.right_, b.y_.right_);
  auto p4 = point(b.x_.left_, b.y_.right_);

  auto q1 = *this * p1;
  auto q2 = *this * p2;
  auto q3 = *this * p3;
  auto q4 = *this * p4;

  int min_x = std::min({q1.x(), q2.x(), q3.x(), q4.x()});
  int max_x = std::max({q1.x(), q2.x(), q3.x(), q4.x()});
  int min_y = std::min({q1.y(), q2.y(), q3.y(), q4.y()});
  int max_y = std::max({q1.y(), q2.y(), q3.y(), q4.y()});

  return box(interval(min_x, max_x), interval(min_y, max_y));
}

rot rot::inverse() const { return rot(cos_, -sin_); }

std::string rot::to_string() const { return "(" + std::to_string(cos_) + ", " + std::to_string(sin_) + ")"; }

} // namespace pivot
