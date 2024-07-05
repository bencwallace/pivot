#pragma once

#include <optional>
#include <tuple>

#include "defines.h"
#include "graphviz.h"
#include "lattice.h"

namespace pivot {

template <int Dim> class walk_node;

template <int Dim>
bool intersect(const walk_node<Dim> *l_walk, const walk_node<Dim> *r_walk, const point<Dim> &l_anchor,
               const point<Dim> &r_anchor, const transform<Dim> &l_symm, const transform<Dim> &r_symm);

template <int Dim> class walk_tree;

template <int Dim> class walk_node {

public:
  static walk_node *pivot_rep(const std::vector<point<Dim>> &steps, walk_node *buf = nullptr);

  /** @brief Create a balanced tree representation of a walk given by a sequence of points.
   *
   * @param steps The steps of the walk.
   * @param buf An optional buffer to use for the tree nodes.
   * @return The root of the walk tree.
   */
  static walk_node *balanced_rep(const std::vector<point<Dim>> &steps, walk_node *buf = nullptr);

  walk_node(const walk_node &w) = delete;
  walk_node(walk_node &&w) = delete;
  walk_node &operator=(const walk_node &w) = delete;

  ~walk_node();

  int id() const { return id_; }

  const box<Dim> &bbox() const { return bbox_; }

  const point<Dim> &endpoint() const { return end_; }

  const transform<Dim> &symm() const { return symm_; }

  walk_node *left() const { return left_; }

  walk_node *right() const { return right_; }

  bool operator==(const walk_node &other) const;

  bool is_leaf() const;

  std::vector<point<Dim>> steps() const;

  void todot(const std::string &path) const;

  walk_node *rotate_left();

  walk_node *rotate_right();

  walk_node *shuffle_up(int id);

  walk_node *shuffle_down();
  // walk_node *shuffle_down(int id);

  bool intersect() const;

  std::pair<walk_node<Dim> *, bool> shuffle_intersect(const transform<Dim> &t, std::optional<bool> was_left_child,
                                                      std::optional<bool> is_left_child);

private:
  int id_;
  int num_sites_;
  walk_node *parent_{};
  walk_node *left_{};
  walk_node *right_{};
  transform<Dim> symm_;
  box<Dim> bbox_;
  point<Dim> end_;

  friend class walk_tree<Dim>;

  walk_node(int id, int num_sites, const transform<Dim> &symm, const box<Dim> &bbox, const point<Dim> &end);

  static walk_node *balanced_rep(std::span<const point<Dim>> steps, int start, const transform<Dim> &glob_symm,
                                 walk_node *buf);

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

  static walk_node create_leaf();
  static walk_node &leaf();

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const;

  template <int D>
  friend bool intersect(const walk_node<D> *l_walk, const walk_node<D> *r_walk, const point<D> &l_anchor,
                        const point<D> &r_anchor, const transform<D> &l_symm, const transform<D> &r_symm);
};

} // namespace pivot
