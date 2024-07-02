#pragma once

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "lattice.h"
#include "walk_base.h"
#include "walk_node.h"

namespace pivot {

class walk_node;

class intersector {
public:
  intersector(int dim, int steps)
      : dim_(dim), l_anchors_(2 * std::ceil(std::log2(steps)), point(dim)),
        r_anchors_(2 * std::ceil(std::log2(steps)), point(dim)),
        l_symms_(2 * std::ceil(std::log2(steps)), transform(dim)),
        r_symms_(2 * std::ceil(std::log2(steps)), transform(dim)) {}

  bool intersect(const walk_node &root) {
    l_anchors_[0] = point(dim_);
    r_anchors_[0] = root.left()->endpoint();
    l_symms_[0] = transform(dim_);
    r_symms_[0] = root.symm();
    return intersect(root.left(), root.right());
  }

private:
  int dim_;
  std::vector<point> l_anchors_;
  std::vector<point> r_anchors_;
  std::vector<transform> l_symms_;
  std::vector<transform> r_symms_;

  bool intersect(const walk_node *l_walk, const walk_node *r_walk, int depth = 0) {
    static box l_box(l_walk->bbox().dim_);
    static box r_box(r_walk->bbox().dim_);

    l_box = l_walk->bbox();
    l_box *= l_symms_[depth];
    l_box += l_anchors_[depth];

    r_box = r_walk->bbox();
    r_box *= r_symms_[depth];
    r_box += r_anchors_[depth];
    r_box &= l_box;

    if (r_box.empty()) {
      return false;
    }

    if (l_walk->num_sites() <= 2 && r_walk->num_sites() <= 2) {
      return true;
    }

    if (l_walk->num_sites() >= r_walk->num_sites()) {
      l_anchors_[depth + 1] = l_walk->left()->endpoint();
      l_anchors_[depth + 1] *= l_symms_[depth];
      l_anchors_[depth + 1] += l_anchors_[depth];
      r_anchors_[depth + 1] = r_anchors_[depth];
      l_symms_[depth + 1] = l_walk->symm();
      l_symms_[depth + 1] *= l_symms_[depth];
      r_symms_[depth + 1] = r_symms_[depth];
      if (intersect(l_walk->right(), r_walk, depth + 1)) {
        return true;
      }

      l_anchors_[depth + 1] = l_anchors_[depth];
      l_symms_[depth + 1] = l_symms_[depth];
      return intersect(l_walk->left(), r_walk, depth + 1);
    } else {
      l_anchors_[depth + 1] = l_anchors_[depth];
      r_anchors_[depth + 1] = r_anchors_[depth];
      l_symms_[depth + 1] = l_symms_[depth];
      r_symms_[depth + 1] = r_symms_[depth];
      if (intersect(l_walk, r_walk->left(), depth + 1)) {
        return true;
      }

      r_anchors_[depth + 1] = r_walk->left()->endpoint();
      r_anchors_[depth + 1] *= r_symms_[depth];
      r_anchors_[depth + 1] += r_anchors_[depth];
      r_symms_[depth + 1] = r_walk->symm();
      r_symms_[depth + 1] *= r_symms_[depth];
      return intersect(l_walk, r_walk->right(), depth + 1);
    }
  }
};

class walk_tree : public walk_base {

public:
  walk_tree(int dim, int num_sites, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  walk_tree(int dim, const std::string &path, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  walk_tree(const std::vector<point> &steps, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  ~walk_tree();

  walk_node *root() const;

  point endpoint() const override;

  bool is_leaf() const;

  /**
   * @brief Find a node by its id
   * @param n node id
   * @note Runs in constant time.
   * @warning This function can only be used on trees initialized with balanced=true. Moreover,
   * it cannot be used on trees currently being transformed (e.g. via rotation).
   * @return reference to the node
   */
  walk_node &find_node(int n);

  bool try_pivot(int n, const transform &r);

  bool rand_pivot() override;

  std::vector<point> steps() const;

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

private:
  int dim_;
  std::unique_ptr<walk_node> root_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> dist_;
  walk_node *buf_;

  static intersector &get_intersector(int dim, int steps) {
    static intersector instance(dim, steps);
    return instance;
  }

  bool intersect() const;
};

} // namespace pivot
