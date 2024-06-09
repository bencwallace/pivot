#include <stdexcept>
#include <vector>

#include "utils.h"

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

template <int Dim> box<Dim> point<Dim>::operator+(const box<Dim> &b) const {
  std::array<interval, Dim> intervals;
  for (int i = 0; i < Dim; ++i) {
    intervals[i] = interval(coords_[i] + b.intervals_[i].left_, coords_[i] + b.intervals_[i].right_);
  }
  return box<Dim>(intervals);
}

template <int Dim> point<Dim> point<Dim>::operator-(const point &p) const {
  std::array<int, Dim> diff;
  for (int i = 0; i < Dim; ++i) {
    diff[i] = coords_[i] - p.coords_[i];
  }
  return point(diff);
}

template <int Dim> point<Dim> operator*(int k, const point<Dim> &p) {
  std::array<int, Dim> coords;
  for (int i = 0; i < Dim; ++i) {
    coords[i] = k * p.coords_[i];
  }
  return point(coords);
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

/* explicit instantiation */

template class point<1>;
template class point<2>;
template class point<3>;
template class point<4>;
template class point<5>;

template point<2> operator*<2>(int k, const point<2> &p);
template point<3> operator*<3>(int k, const point<3> &p);
template point<4> operator*<4>(int k, const point<4> &p);
template point<5> operator*<5>(int k, const point<5> &p);

template std::size_t point_hash::operator()<2>(const point<2> &p) const;
template std::size_t point_hash::operator()<3>(const point<3> &p) const;
template std::size_t point_hash::operator()<4>(const point<4> &p) const;
template std::size_t point_hash::operator()<5>(const point<5> &p) const;

} // namespace pivot
