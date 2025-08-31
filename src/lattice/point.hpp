#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

template <int Dim, bool Simd> point<Dim, Simd>::point(const std::array<int, Dim> &coords) : coords_(coords) {}

template <int Dim, bool Simd> point<Dim, Simd> point<Dim, Simd>::unit(int i) {
  point<Dim, Simd> result;
  result.coords_[i] = 1;
  return result;
}

template <int Dim, bool Simd> int point<Dim, Simd>::operator[](int i) const { return coords_[i]; }

template <int Dim, bool Simd> bool point<Dim, Simd>::operator==(const point &p) const {
  // Faster than using std::equal, at least for small arrays
  // https://stackoverflow.com/questions/39262496/why-is-stdequal-much-slower-than-a-hand-rolled-loop-for-two-small-stdarray
  for (int i = 0; i < Dim; ++i) {
    if (coords_[i] != p.coords_[i]) {
      return false;
    }
  }
  return true;
}

template <int Dim, bool Simd> bool point<Dim, Simd>::operator!=(const point &p) const { return !(*this == p); }

template <int Dim, bool Simd> point<Dim, Simd> point<Dim, Simd>::operator+(const point<Dim, Simd> &p) const {
  std::array<int, Dim> sum;
  for (int i = 0; i < Dim; ++i) {
    sum[i] = coords_[i] + p.coords_[i];
  }
  return point(sum);
}

template <int Dim, bool Simd> point<Dim, Simd> point<Dim, Simd>::operator-(const point &p) const {
  std::array<int, Dim> diff;
  for (int i = 0; i < Dim; ++i) {
    diff[i] = coords_[i] - p.coords_[i];
  }
  return point(diff);
}

template <int Dim, bool Simd> point<Dim, Simd> &point<Dim, Simd>::operator*=(int k) {
  for (int i = 0; i < Dim; ++i) {
    coords_[i] *= k;
  }
  return *this;
}

template <int Dim, bool Simd> int point<Dim, Simd>::norm() const {
  int sum = 0;
  for (int i = 0; i < Dim; ++i) {
    sum += coords_[i] * coords_[i];
  }
  return sum;
}

template <int Dim, bool Simd> std::string point<Dim, Simd>::to_string() const {
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

} // namespace pivot
