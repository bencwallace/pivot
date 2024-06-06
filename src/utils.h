#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <vector>

#include <string>

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
template <int Dim> class point {

public:
  point() = default;

  point(const std::array<int, Dim> &coords) : coords_(coords) {}

  /**
   * @brief Returns the unit vector e_i.
   *
   * @param i The index of the unit vector. Must be in [0, Dim).
   */
  static point unit(int i) {
    point<Dim> result;
    result.coords_[i] = 1;
    return result;
  }

  int operator[](int i) const { return coords_[i]; }

  bool operator==(const point &p) const { return coords_ == p.coords_; }

  bool operator!=(const point &p) const { return coords_ != p.coords_; }

  /** @brief Vector addition of points */
  point operator+(const point &p) const {
    std::array<int, Dim> sum;
    for (int i = 0; i < Dim; ++i) {
      sum[i] = coords_[i] + p.coords_[i];
    }
    return point(sum);
  }

  /** @brief Action of a point (understood as a vector) on a box */
  box<Dim> operator+(const box<Dim> &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(coords_[i] + b.intervals_[i].left_, coords_[i] + b.intervals_[i].right_);
    }
    return box<Dim>(intervals);
  }

  /** @brief Vector subtraction of points */
  point operator-(const point &p) const {
    std::array<int, Dim> diff;
    for (int i = 0; i < Dim; ++i) {
      diff[i] = coords_[i] - p.coords_[i];
    }
    return point(diff);
  }

  /** @brief Scalar multiplication of a point */
  friend point operator*(int k, const point &p) {
    std::array<int, Dim> coords;
    for (int i = 0; i < Dim; ++i) {
      coords[i] = k * p.coords_[i];
    }
    return point(coords);
  }

  int norm() const {
    int sum = 0;
    for (int i = 0; i < Dim; ++i) {
      sum += coords_[i] * coords_[i];
    }
    return sum;
  }

  /** @brief Returns the string of the form "({coords_[0]}, ..., {coords_[Dim - 1]})" */
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

template <typename S, typename T, T Dim> point(std::array<S, Dim>) -> point<Dim>;

/** @brief Represents a Dim-dimensional box. */
template <int Dim> struct box {
  std::array<interval, Dim> intervals_;

  box() = delete;

  box(const std::array<interval, Dim> &intervals) : intervals_(intervals) {}

  /**
   * @brief Constructs the smallest box containing a sequence of Dim-dimensional points.
   *
   * The resulting box is "anchored" to the standard unit vector along the first coordinate axis (e0),
   * in the sense that the input sequences of points (p0, ...) is effectively translated by e0 - p0
   * prior to the box's construction. In other words, the box will always have a vertex at e0.
   */
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

  bool operator==(const box &b) const { return intervals_ == b.intervals_; }

  bool operator!=(const box &b) const { return intervals_ != b.intervals_; }

  interval operator[](int i) const { return intervals_[i]; }

  bool empty() const {
    return std::any_of(intervals_.begin(), intervals_.end(), [](const interval &i) { return i.empty(); });
  }

  /**
   * @brief Returns the "union" of two boxes.
   *
   * The union here is not to be understood as the set-theoretic union, but rather as the minimal
   * bounding box containing both input boxes.
   */
  box operator+(const box &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(std::min(intervals_[i].left_, b.intervals_[i].left_),
                              std::max(intervals_[i].right_, b.intervals_[i].right_));
    }
    return box(intervals);
  }

  /**
   * @brief Returns the intersection of two boxes.
   *
   * The intersection is the set-theoretic intersection or, equivalently, the maximal box contained
   * in both input boxes.
   */
  box operator*(const box &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      intervals[i] = interval(std::max(intervals_[i].left_, b.intervals_[i].left_),
                              std::min(intervals_[i].right_, b.intervals_[i].right_));
    }
    return box(intervals);
  }

  /** @brief Returns the string of the form "{intervals_[0]} x ... x {intervals[Dim - 1]}". */
  std::string to_string() const {
    std::string s = "";
    for (int i = 0; i < Dim - 1; ++i) {
      s += intervals_[i].to_string() + " x ";
    }
    s += intervals_[Dim - 1].to_string();
    return s;
  }
};

template <typename S, typename T, S Dim, T N> box(std::array<point<Dim>, N>) -> box<Dim>;

struct point_hash {
  int num_steps_;

  point_hash(int num_steps);

  // This hashing method, which exploits the known range of values that can be taken by the
  // sequence of points in a walk, ppears to result in better performance than other methods tested.
  template <int Dim> std::size_t operator()(const point<Dim> &p) const {
    std::size_t hash = 0;
    for (int i = 0; i < Dim; ++i) {
      hash = num_steps_ + p[i] + 2 * num_steps_ * hash;
    }
    return hash;
  }
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
  transform() {
    for (int i = 0; i < Dim; ++i) {
      perm_[i] = i;  // trivial permutation
      signs_[i] = 1; // standard orientations (no flips)
    }
  }

  // TODO: this should be private
  transform(const std::array<int, Dim> &perm, const std::array<int, Dim> &signs) : perm_(perm), signs_(signs) {}

  /**
   * @brief Constructs a "pivot" transformation from two input points.
   *
   * The two input points are assumed to differ by a (possibly flipped) standard unit vector.
   * The first input point is viewed as an anchor about which the standard unit vector for
   * the first coordinate axis should be pivoted in order to obtain this difference.
   *
   * Note that the resulting pivot transformation is not uniquely defined by the inputs.
   */
  transform(const point<Dim> &p, const point<Dim> &q) : transform() {
    // The input points should differ by 1 in a single coordinate. Start by finding this coordinate or fail.
    point<Dim> diff = q - p;
    int idx = -1;
    for (int i = 0; i < Dim; ++i) {
      if (std::abs(diff[i]) == 1) {
        if (idx == -1) {
          idx = i;
        } else { // a differing coordinate has already been found
          throw std::invalid_argument("Points are not adjacent");
        }
      }
    }
    if (idx == -1) {
      throw std::invalid_argument("Points are not adjacent");
    }

    // Construct the transform by modifying the identity transform.
    perm_[0] = idx;
    perm_[idx] = 0;
    signs_[0] = -diff[idx]; // not strictly necessary
    signs_[idx] = diff[idx];
  }

  /** @brief Produce a uniformly random transfom.*/
  template <typename Gen> static transform rand(Gen &gen) {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[i] = i;
      signs[i] = 2 * (std::rand() % 2) - 1;
    }
    std::shuffle(perm.begin(), perm.end(), gen);
    return transform(perm, signs);
  }

  static transform rand() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return rand(gen);
  }

  bool operator==(const transform &t) const { return perm_ == t.perm_ && signs_ == t.signs_; }

  /** @brief Transforms a point. */
  point<Dim> operator*(const point<Dim> &p) const {
    std::array<int, Dim> coords;
    for (int i = 0; i < Dim; ++i) {
      coords[i] = signs_[i] * p[perm_[i]];
    }
    return point<Dim>(coords);
  }

  /**
   * @brief Composes two transforms.
   *
   * @details Given matrix representations (see @ref transform_details) S1 P1 and S2 P2 of *this and t, respectively,
   * the composed transformation acts on a standard unit vector e(i) by producing
   *
   *    S1 P1 S2 P2 e(i) == S1 P1 S2 e(P2(i))
   *                     == S2(P2(i)) S1 P1 e(P2(i))
   *                     == S2(P2(i)) S1 e(P1(P2(i)))
   *                     == S1(P1(P2(i))) S2(P2(i)) e(P1(P2(i)))
   *
   * from which the permutation and signs of the composed transformation can be read off.
   */
  transform operator*(const transform &t) const {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[i] = perm_[t.perm_[i]];
      signs[perm[i]] = signs_[perm[i]] * t.signs_[t.perm_[i]];
    }
    return transform(perm, signs);
  }

  /**
   * @brief Action of a transform on a box.
   *
   * @details Using the representation of the inverse of a transform from @ref transform::inverse we
   * see that the pre-image of a point x under such a transform has i-th component given by
   *
   *    x[i] == S(P(i)) x[P(i)]
   *
   * Given a box B made up of intervals [a[i], b[i]], the image SP(B) of the box consists of points
   * whose pre-images have i-th components between a[i] and b[i] for each i. This is equivalent to saying
   *
   *    a[i] \le S(P(i)) x[P(i)] \le b[i]
   *
   * which means the P(i)-th interval from which SP(B) is constructed has bounds S(P(i)) a[i] and
   * S(P(i)) b[i].
   */
  box<Dim> operator*(const box<Dim> &b) const {
    std::array<interval, Dim> intervals;
    for (int i = 0; i < Dim; ++i) {
      int x = signs_[perm_[i]] * b.intervals_[i].left_;
      int y = signs_[perm_[i]] * b.intervals_[i].right_;
      intervals[perm_[i]] = interval(std::min(x, y), std::max(x, y));
    }
    return box<Dim>(intervals);
  }

  /**
   * @brief Returns the inverse transform.
   *
   * @details Writing the transforms as SP (see @ref transform_details) and using the fact that S is its own inverse,
   * the inverse transform acts on a standard unit vector e(P(i)) by producing
   *
   *    (S P)^{-1} e(P(i)) == P^{-1} S^{-1} e(P(i))
   *                       == P^{-1} S e(P(i))
   *                       == S(P(i)) P^{-1} e(P(i))
   *                       == S(P(i)) e(i)
   *
   * from which the permutation and signs of the inverse transform can be read off.
   */
  transform inverse() const {
    std::array<int, Dim> perm;
    std::array<int, Dim> signs;
    for (int i = 0; i < Dim; ++i) {
      perm[perm_[i]] = i;
      signs[i] = signs_[perm_[i]];
    }
    return transform(perm, signs);
  }

  /**
   * @brief Returns the matrix representation of the transform.
   *
   * @details Writing the transform as SP (see @ref transform_details), the matrix representation
   * can be read off from
   *
   *    S P e(i) = S e(P(i)) = S(P(i)) e(P(i))
   *
   * which means the i-th column of the matrix has value S(P(i)) in the P(i)-th row and zeros elsewhere.
   */
  std::array<std::array<int, Dim>, Dim> to_matrix() const {
    std::array<std::array<int, Dim>, Dim> matrix = {};
    for (int i = 0; i < Dim; ++i) {
      matrix[perm_[i]][i] = signs_[perm_[i]];
    }
    return matrix;
  }

  /**
   * @brief Returns a string representation of the transform.
   *
   * Represents the matrix corresponding to the transform as a nested list.
   */
  std::string to_string() const {
    auto matrix = to_matrix();
    std::string s = "[";
    for (int i = 0; i < Dim; ++i) {
      s += "[";
      for (int j = 0; j < Dim - 1; ++j) {
        s += std::to_string(matrix[i][j]) + ", ";
      }
      s += std::to_string(matrix[i][Dim - 1]) + "]";
      if (i < Dim - 1) {
        s += ", ";
      }
    }
    s += "]";
    return s;
  }

private:
  std::array<int, Dim> perm_;
  std::array<int, Dim> signs_;
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
