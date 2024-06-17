#pragma once

#include <cstddef>
#include <type_traits>

#include "lattice.h"

namespace pivot {

template <typename Derived> class Expr {

public:
  static constexpr bool is_leaf = true;

  int operator[](size_t i) const { return static_cast<const Derived *>(this)->operator[](i); }
};

template <typename Derived> class PointExpr : public Expr<Derived> {};

template <typename Derived> class BoxExpr : public Expr<Derived> {

public:
  int left(size_t i) const { return static_cast<const Derived *>(this)->left(i); }
  int right(size_t i) const { return static_cast<const Derived *>(this)->right(i); }
};

template <typename E1, typename E2> class PointBoxSum : public BoxExpr<PointBoxSum<E1, E2>> {

public:
  static constexpr bool is_leaf = false;

  PointBoxSum(const E1 &p, const E2 &b) : p_(p), b_(b) {}

  int left(size_t i) const { return p_[i] + b_[i].left_; }
  int right(size_t i) const { return p_[i] + b_[i].right_; }

private:
  typename std::conditional_t<E1::is_leaf, const E1 &, const E1> p_;
  typename std::conditional_t<E2::is_leaf, const E2 &, const E2> b_;
};

template <typename E1, typename E2> PointBoxSum<E1, E2> operator+(const PointExpr<E1> &p, const BoxExpr<E2> &b) {
  return PointBoxSum<E1, E2>(*static_cast<const E1 *>(&p), *static_cast<const E2 *>(&b));
}

template <typename E1, typename E2> class BoxPointDiff : public BoxExpr<BoxPointDiff<E1, E2>> {

public:
  static constexpr bool is_leaf = false;

  BoxPointDiff(const E1 &b, const E2 &p) : b_(b), p_(p) {}

  int left(size_t i) const { return b_[i].left_ - p_[i]; }
  int right(size_t i) const { return b_[i].right_ - p_[i]; }

private:
  typename std::conditional_t<E1::is_leaf, const E1 &, const E1> b_;
  typename std::conditional_t<E2::is_leaf, const E2 &, const E2> p_;
};

template <typename E1, typename E2> BoxPointDiff<E1, E2> operator-(const BoxExpr<E1> &b, const PointExpr<E2> &p) {
  return BoxPointDiff<E1, E2>(*static_cast<const E1 *>(&b), *static_cast<const E2 *>(&p));
}

template <typename E1, typename E2> class PointSum : public PointExpr<PointSum<E1, E2>> {

public:
  static constexpr bool is_leaf = false;

  PointSum(const E1 &left, const E2 &right) : left_(left), right_(right) {}

  int operator[](size_t i) const { return left_[i] + right_[i]; }

private:
  typename std::conditional_t<E1::is_leaf, const E1 &, const E1> left_;
  typename std::conditional_t<E2::is_leaf, const E2 &, const E2> right_;
};

template <typename E1, typename E2> PointSum<E1, E2> operator+(const PointExpr<E1> &left, const PointExpr<E2> &right) {
  return PointSum<E1, E2>(*static_cast<const E1 *>(&left), *static_cast<const E2 *>(&right));
}

} // namespace pivot
