#pragma once

#include "defines.h"
#include "graphviz.h"
#include "lattice.h"

namespace pivot {

class walk_node;

bool intersect(const walk_node *l_walk, const walk_node *r_walk, const point &l_anchor, const point &r_anchor,
               const transform &l_symm, const transform &r_symm, size_t depth = 0);

class walk_tree;

class walk_node {

public:
  static walk_node *pivot_rep(const std::vector<point> &steps, walk_node *buf = nullptr);

  /** @brief Create a balanced tree representation of a walk given by a sequence of points.
   *
   * @param steps The steps of the walk.
   * @param buf An optional buffer to use for the tree nodes.
   * @return The root of the walk tree.
   */
  static walk_node *balanced_rep(const std::vector<point> &steps, walk_node *buf = nullptr);

  walk_node(const walk_node &w) = delete;
  walk_node(walk_node &&w) = delete;
  walk_node &operator=(const walk_node &w) = delete;

  ~walk_node();

  int id() const { return id_; }

  const box &bbox() const { return bbox_; }

  const point &endpoint() const { return end_; }

  const transform &symm() const { return symm_; }

  walk_node *left() const { return left_; }

  walk_node *right() const { return right_; }

  bool operator==(const walk_node &other) const;

  bool is_leaf() const;

  std::vector<point> steps() const;

  void todot(const std::string &path) const;

  walk_node *rotate_left();

  walk_node *rotate_right();

  walk_node *shuffle_up(int id);

  walk_node *shuffle_down();

private:
  int id_;
  int num_sites_;
  walk_node *parent_{};
  walk_node *left_{};
  walk_node *right_{};
  transform symm_;
  box bbox_;
  point end_;

  friend class walk_tree;

  walk_node(int id, int num_sites, const transform &symm, const box &bbox, const point &end);

  static walk_node *balanced_rep(std::span<const point> steps, int start, const transform &glob_symm, walk_node *buf);

  void set_left(walk_node *left) {
    left_ = left;
    if (left != nullptr) {
      left->parent_ = this;
    }
  }

  void set_right(walk_node *right) {
    right_ = right;
    if (right != nullptr) {
      right->parent_ = this;
    }
  }

  void merge();

  static walk_node create_leaf(int dim);
  static walk_node &leaf(int dim);

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const;

  friend bool intersect(const walk_node *l_walk, const walk_node *r_walk, const point &l_anchor, const point &r_anchor,
                        const transform &l_symm, const transform &r_symm, size_t depth);
};

} // namespace pivot
