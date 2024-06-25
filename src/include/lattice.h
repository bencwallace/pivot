#pragma once

#include <algorithm>
#include <limits>
#include <random>
#include <span>
#include <string>
#include <vector>

#include <boost/operators.hpp>

#include "defines.h"

namespace pivot {

template <int Dim> struct box;

/**
 * @brief Represents a closed interval [left_, right_].
 *
 * If left_ > right_, the interval is empty.
 */
struct interval {
  int left_;
  int right_;

  interval();
  interval(int left, int right);

  bool operator==(const interval &i) const;

  bool operator!=(const interval &i) const;

  bool empty() const;

  std::string to_string() const;
};

/**
 * @brief Represents a Dim-dimensional point.
 */
template <int Dim> class point : boost::multipliable<point<Dim>, int> {

public:
  point() = default;

  explicit point(const std::array<int, Dim> &coords);

  /**
   * @brief Returns the unit vector e_i.
   *
   * @param i The index of the unit vector. Must be in [0, Dim).
   */
  static point unit(int i);

  int operator[](int i) const;

  bool operator==(const point &p) const;

  bool operator!=(const point &p) const;

  /** @brief Vector addition of points */
  point operator+(const point &p) const;

  /** @brief Vector subtraction of points */
  point operator-(const point &p) const;

  /** @brief Scalar multiplication of a point */
  point &operator*=(int k);

  int norm() const;

  /** @brief Returns the string of the form "({coords_[0]}, ..., {coords_[Dim - 1]})" */
  std::string to_string() const;

private:
  std::array<int, Dim> coords_{};
};

template <typename S, typename T, T Dim> point(std::array<S, Dim>) -> point<Dim>;

/** @brief Represents a Dim-dimensional box. */
template <int Dim> struct box : boost::additive<box<Dim>, point<Dim>> {
  std::array<interval, Dim> intervals_;

  box() = delete;

  box(const std::array<interval, Dim> &intervals);

  /**
   * @brief Constructs the smallest box containing a sequence of Dim-dimensional points.
   *
   * The resulting box is "anchored" to the standard unit vector along the first coordinate axis (e0),
   * in the sense that the input sequences of points (p0, ...) is effectively translated by e0 - p0
   * prior to the box's construction. In other words, the box will always have a vertex at e0.
   */
  box(std::span<const point<Dim>> points);

  bool operator==(const box &b) const;

  bool operator!=(const box &b) const;

  interval operator[](int i) const;

  bool empty() const;

  /** @brief Action of a point (understood as a vector) on a box */
  box<Dim> &operator+=(const point<Dim> &b);

  box<Dim> &operator-=(const point<Dim> &b);

  /**
   * @brief Returns the "union" of two boxes.
   *
   * The union here is not to be understood as the set-theoretic union, but rather as the minimal
   * bounding box containing both input boxes.
   */
  box operator|(const box &b) const;

  /**
   * @brief Returns the intersection of two boxes.
   *
   * The intersection is the set-theoretic intersection or, equivalently, the maximal box contained
   * in both input boxes.
   */
  box operator&(const box &b) const;

  /** @brief Returns the string of the form "{intervals_[0]} x ... x {intervals[Dim - 1]}". */
  std::string to_string() const;
};

template <typename S, typename T, S Dim, T N> box(std::array<point<Dim>, N>) -> box<Dim>;

struct point_hash {
  int num_steps_;

  point_hash(int num_steps);

  // This hashing method, which exploits the known range of values that can be taken by the
  // sequence of points in a walk, ppears to result in better performance than other methods tested.
  template <int Dim> std::size_t operator()(const point<Dim> &p) const;
};

/**
 * @brief Represents a transformation from the symmetry group of the cubic lattice.
 *
 * This symmetry group can be generated by all d! permutations of the coordinate axes,
 * together with the 2^d choices of orientations of these axes. A permutation is represented
 * by an array permuting the sequence {0, ..., Dim - 1} and a choice of orientations
 * by a Dim-dimensional array of sign flips (plus or minus 1).
 *
 * @anchor transform_details
 * @details Such a transformation can be represented by the matrix product S P of a
 * permutation matrix P and a diagonal matrix S with diagonal entries equal to plus or
 * minus one. We denote the action of the permutation on an integer by P(i) and the diagonal
 * entries of S by S(i).
 */
template <int Dim> class transform {

public:
  /** @brief Constructs the identity transformation. */
  transform();

  // TODO: this should be private
  transform(const std::array<int, Dim> &perm, const std::array<int, Dim> &signs);

  /**
   * @brief Constructs a "pivot" transformation from two input points.
   *
   * The two input points are assumed to differ by a (possibly flipped) standard unit vector.
   * The first input point is viewed as an anchor about which the standard unit vector for
   * the first coordinate axis should be pivoted in order to obtain this difference.
   *
   * Note that the resulting pivot transformation is not uniquely defined by the inputs.
   */
  transform(const point<Dim> &p, const point<Dim> &q);

  /** @brief Produce a uniformly random transfom.*/
  template <typename Gen> static transform rand(Gen &gen) {
    static std::bernoulli_distribution flip_;

    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[i] = i;
      signs[i] = 2 * flip_(gen) - 1;
    }
    std::shuffle(perm.begin(), perm.end(), gen);
    return transform(perm, signs);
  }

  static transform rand();

  bool operator==(const transform &t) const;

  /** @brief Transforms a point. */
  point<Dim> operator*(const point<Dim> &p) const;

  /**
   * @brief Composes two transforms.
   *
   * @details Given matrix representations (see @ref transform_details) S1 P1 and S2 P2 of *this and t, respectively,
   * the composed transformation acts on a standard unit vector e(i) by producing
   *
   * \f{align}{
   *    S1 P1 S2 P2 e(i) &= S1 P1 S2 e(P2(i)) \\
   *                     &= S2(P2(i)) S1 P1 e(P2(i)) \\
   *                     &= S2(P2(i)) S1 e(P1(P2(i))) \\
   *                     &= S1(P1(P2(i))) S2(P2(i)) e(P1(P2(i))) \\
   * \f}
   *
   * from which the permutation and signs of the composed transformation can be read off.
   */
  transform operator*(const transform &t) const;

  /**
   * @brief Action of a transform on a box.
   *
   * @details Using the representation of the inverse of a transform from @ref transform::inverse we
   * see that the pre-image of a point x under such a transform has i-th component given by
   *
   * \f[ x[i] = S(P(i)) x[P(i)] \f]
   *
   * Given a box B made up of intervals [a[i], b[i]], the image SP(B) of the box consists of points
   * whose pre-images have i-th components between a[i] and b[i] for each i. This is equivalent to saying
   *
   * \f[ a[i] \le S(P(i)) x[P(i)] \le b[i] \f]
   *
   * which means the P(i)-th interval from which SP(B) is constructed has bounds S(P(i)) a[i] and
   * S(P(i)) b[i].
   */
  box<Dim> operator*(const box<Dim> &b) const;

  /**
   * @brief Returns the inverse transform.
   *
   * @details Writing the transforms as SP (see @ref transform_details) and using the fact that S is its own inverse,
   * the inverse transform acts on a standard unit vector e(P(i)) by producing
   *
   * \f{align}{
   *    (S P)^{-1} e(P(i)) &= P^{-1} S^{-1} e(P(i)) \\
   *                       &= P^{-1} S e(P(i)) \\
   *                       &= S(P(i)) P^{-1} e(P(i)) \\
   *                       &= S(P(i)) e(i) \\
   * \f}
   *
   * from which the permutation and signs of the inverse transform can be read off.
   */
  transform inverse() const;

  /**
   * @brief Returns the matrix representation of the transform.
   *
   * @details Writing the transform as SP (see @ref transform_details), the matrix representation
   * can be read off from
   *
   * \f[S P e(i) = S e(P(i)) = S(P(i)) e(P(i))\f]
   *
   * which means the i-th column of the matrix has value S(P(i)) in the P(i)-th row and zeros elsewhere.
   */
  std::array<std::array<int, Dim>, Dim> to_matrix() const;

  /**
   * @brief Returns a string representation of the transform.
   *
   * Represents the matrix corresponding to the transform as a nested list.
   */
  std::string to_string() const;

private:
  std::array<int, Dim> perm_;
  std::array<int, Dim> signs_;
};

} // namespace pivot
