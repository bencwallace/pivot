#include <stdexcept>
#include <vector>

#include "utils.h"

namespace pivot {

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

bool interval::empty() const { return left_ > right_; }

std::string interval::to_string() const { return "[" + std::to_string(left_) + ", " + std::to_string(right_) + "]"; }

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

rot::rot(point<2> p, point<2> q) {
  auto dx = q[0] - p[0];
  auto dy = q[1] - p[1];
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

point<2> rot::operator*(const point<2> &p) const {
  return point<2>({cos_ * p[0] - sin_ * p[1], sin_ * p[0] + cos_ * p[1]});
}

rot rot::operator*(const rot &r) const { return rot(cos_ * r.cos_ - sin_ * r.sin_, sin_ * r.cos_ + cos_ * r.sin_); }

box<2> rot::operator*(const box<2> &b) const {
  auto p1 = point<2>({b[0].left_, b[1].left_});
  auto p2 = point<2>({b[0].right_, b[1].left_});
  auto p3 = point<2>({b[0].right_, b[1].right_});
  auto p4 = point<2>({b[0].left_, b[1].right_});

  auto q1 = *this * p1;
  auto q2 = *this * p2;
  auto q3 = *this * p3;
  auto q4 = *this * p4;

  int min_x = std::min({q1[0], q2[0], q3[0], q4[0]});
  int max_x = std::max({q1[0], q2[0], q3[0], q4[0]});
  int min_y = std::min({q1[1], q2[1], q3[1], q4[1]});
  int max_y = std::max({q1[1], q2[1], q3[1], q4[1]});

  return box<2>({interval(min_x, max_x), interval(min_y, max_y)});
}

rot rot::inverse() const { return rot(cos_, -sin_); }

std::string rot::to_string() const { return "(" + std::to_string(cos_) + ", " + std::to_string(sin_) + ")"; }

} // namespace pivot
