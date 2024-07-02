#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"

namespace pivot {

point::point(int dim) : dim_(dim), coords_(dim, 0) {}

point::point(std::vector<int> &&coords) : dim_(coords.size()), coords_(std::move(coords)) {}

point::point(std::initializer_list<int> coords) : dim_(coords.size()), coords_(coords) {}

point point::unit(int dim, int i) {
  point result(dim);
  result.coords_[i] = 1;
  return result;
}

int point::operator[](int i) const { return coords_[i]; }

bool point::operator==(const point &p) const { return coords_ == p.coords_; }

bool point::operator!=(const point &p) const { return coords_ != p.coords_; }

point &point::operator+=(const point &p) {
  for (int i = 0; i < dim_; ++i) {
    coords_[i] += p.coords_[i];
  }
  return *this;
}

point &point::operator-=(const point &p) {
  for (int i = 0; i < dim_; ++i) {
    coords_[i] -= p.coords_[i];
  }
  return *this;
}

point &point::operator*=(int k) {
  for (int i = 0; i < dim_; ++i) {
    coords_[i] *= k;
  }
  return *this;
}

int point::dim() const { return dim_; }

int point::norm() const {
  int sum = 0;
  for (int i = 0; i < dim_; ++i) {
    sum += coords_[i] * coords_[i];
  }
  return sum;
}

std::string point::to_string() const {
  std::string s = "(";
  for (int i = 0; i < dim_ - 1; ++i) {
    s += std::to_string(coords_[i]) + ", ";
  }
  s += std::to_string(coords_[dim_ - 1]);
  if (dim_ == 1) {
    s += ",)";
  } else {
    s += ")";
  }
  return s;
}

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

std::size_t point_hash::operator()(const point &p) const {
  std::size_t hash = 0;
  for (int i = 0; i < p.dim(); ++i) {
    hash = num_steps_ + p[i] + 2 * num_steps_ * hash;
  }
  return hash;
}

} // namespace pivot
