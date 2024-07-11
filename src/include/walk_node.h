#pragma once

#include <optional>

#include "defines.h"
#include "graphviz.h"
#include "lattice.h"

namespace pivot {

/* FORWARD REFERENCES */

template <int Dim> class walk_node;

template <int Dim>
bool intersect(const walk_node<Dim> *l_walk, const walk_node<Dim> *r_walk, const point<Dim> &l_anchor,
               const point<Dim> &r_anchor, const transform<Dim> &l_symm, const transform<Dim> &r_symm);

template <int Dim> class walk_tree;

/* WALK NODE */

/** @brief Represents a node in a walk tree. */
template <int Dim> class walk_node {

public:
  /* CONSTRUCTORS, DESTRUCTOR */

  /** @brief Returns the root of a walk tree for the pivot representation of sequence of lattice sites.
   *
   * @param steps The lattice sites of the walk. Must have size at least 2 (single step).
   * @param buf An optional buffer in which to store the tree nodes.
   *
   * @return The root of the walk tree.
   */
  static walk_node *pivot_rep(const std::vector<point<Dim>> &steps, walk_node *buf = nullptr);

  /** @brief Returns the root of a walk tree for the balanced representation of a walk given by a sequence of points.
   *
   * @param steps The lattice sites of the walk. Must have size at least 2 (single step).
   * @param buf An optional buffer in which to store the tree nodes.
   *
   * @return The root of the walk tree.
   */
  static walk_node *balanced_rep(const std::vector<point<Dim>> &steps, walk_node *buf = nullptr);

  /** @brief Copies the given node but none of the nodes it links to. */
  walk_node(const walk_node &w) = default;

  walk_node(walk_node &&w) = delete;
  walk_node &operator=(const walk_node &w) = delete;

  /** @brief Deallocates the node but none of the nodes it links to. */
  ~walk_node();

  /* GETTERS, SETTERS, SIMPLE UTILITIES */

  int id() const { return id_; }

  const box<Dim> &bbox() const { return bbox_; }

  const point<Dim> &endpoint() const { return end_; }

  const transform<Dim> &symm() const { return symm_; }

  walk_node *left() const { return left_; }

  walk_node *right() const { return right_; }

  /** @brief Checks if two nodes hold the same data, but not whether they are part of the same tree. */
  bool operator==(const walk_node &other) const;

  bool is_leaf() const;

  /* PRIMITIVE OPERATIONS (see Clisby (2010), Section 2.5) */

  /** @brief Merge the left and right subtrees of the current node. */
  void merge();

  /**
   * @brief Perform a left rotation.
   *
   * @param set_parent Whether to modify links between nodes bidirectionally. Should typically only be set to false
   * when called from shuffle_intersect.
   *
   * @return The new root of the subtree.
   */
  walk_node *rotate_left(bool set_parent = true);

  /**
   * @brief Perform a right rotation.
   *
   * @param set_parent Whether to modify links between nodes bidirectionally. Should typically only be set to false
   * when called from shuffle_intersect.
   *
   * @return The new root of the subtree.
   */
  walk_node *rotate_right(bool set_parent = true);

  /* USER-LEVEL OPERATIONS (see Clisby (2010), Section 2.6) */

  /**
   * @brief Shuffle the node with the given ID up to the root of the tree.
   *
   * @param id The id of the node to shuffle up.
   *
   * @return The new root of the tree.
   */
  walk_node *shuffle_up(int id);

  /**
   * @brief Shuffle the current node down to the appropriate level in a balanced tree.
   *
   * @param id The id of the node to shuffle down.
   *
   * @return The new root of the tree.
   */
  walk_node *shuffle_down();

  /**
   * @brief Checks if the given transform applied at the current node creates an intersection via a bottom-up algorithm.
   *
   * @param t The given transform.
   * @param is_left_child Whether the current node is the left child of its parent.
   *
   * @return Whether the transform creates an intersection.
   */
  bool shuffle_intersect(const transform<Dim> &t, std::optional<bool> is_left_child);

  /**
   * @brief Checks if the current walk has an intersection via a top-down algorithm.
   *
   * @return Whether the transform creates an intersection.
   */
  bool intersect() const;

  /* OTHER FUNCTIONS */

  std::vector<point<Dim>> steps() const;

  void todot(const std::string &path) const;

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

  /* CONVENIENCE METHODS */

  walk_node(int id, int num_sites, const transform<Dim> &symm, const box<Dim> &bbox, const point<Dim> &end);

  void set_left(walk_node *left) {
    left_ = left;
    if (left != nullptr && !left->is_leaf()) {
      left->parent_ = this;
    }
  }

  void set_right(walk_node *right) {
    right_ = right;
    if (right != nullptr && !right->is_leaf()) {
      right->parent_ = this;
    }
  }

  std::optional<bool> is_left_child() const {
    if (parent_ == nullptr) {
      return std::nullopt;
    }
    return parent_->left_ == this;
  }

  static walk_node create_leaf();
  static walk_node &leaf();

  /* RECURSION HELPERS */

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const;

  // recursive helper
  static walk_node *balanced_rep(std::span<const point<Dim>> steps, int start, const transform<Dim> &glob_symm,
                                 walk_node *buf);

  bool shuffle_intersect(const transform<Dim> &t, std::optional<bool> was_left_child,
                         std::optional<bool> is_left_child);

  template <int D>
  friend bool intersect(const walk_node<D> *l_walk, const walk_node<D> *r_walk, const point<D> &l_anchor,
                        const point<D> &r_anchor, const transform<D> &l_symm, const transform<D> &r_symm);
};

} // namespace pivot
