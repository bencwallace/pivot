#pragma once

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk_node;

/** @brief Represents an entire saw-tree (as per Clisby's 2010 paper). */
template <int Dim> class walk_tree : public walk_base<Dim> {

public:
  /* CONSTRUCTORS, DESTRUCTOR */

  /**
   * @brief Construct a walk tree representing a straight line on the given number of lattice sites.
   *
   * @param num_sites Number of lattice sites. Must be at least 2 (single step).
   * @param seed Random seed. Not used in construction of the initial tree, but rather to seed the random
   * number generator used for pivoting. If not provided, a random seed is chosen.
   * @param balanced Whether to construct the tree using a balanced representation (the deafult) or the
   * (imbalanced) "pivot representation".
   *
   * @warning It is not recommended to set balanced=false.
   */
  walk_tree(int num_sites, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  /**
   * @brief Load a walk tree from a given checkpoint.
   *
   * @param path Path to the checkpoint file. Should be in a CSV file in which each line is a lattice site,
   * represented as a comma-separated list of integers.
   * @param seed Random seed. Not used in construction of the initial tree, but rather to seed the random
   * number generator used for pivoting. If not provided, a random seed is chosen.
   * @param balanced Whether to construct the tree using a balanced representation (the deafult) or the
   * (imbalanced) "pivot representation".
   *
   * @warning It is not recommended to set balanced=false.
   */
  walk_tree(const std::string &path, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  /**
   * @brief Construct a walk tree from a given sequence of lattice sites.
   *
   * @param steps Sequence of lattice sites. Must be at least 2 (single step).
   * @param seed Random seed. Not used in construction of the initial tree, but rather to seed the random
   * number generator used for pivoting. If not provided, a random seed is chosen.
   * @param balanced Whether to construct the tree using a balanced representation (the deafult) or the
   * (imbalanced) "pivot representation".
   *
   * @warning It is not recommended to set balanced=false.
   */
  walk_tree(const std::vector<point<Dim>> &steps, std::optional<unsigned int> seed = std::nullopt,
            bool balanced = true);

  /**@brief Deallocates the entire tree and every node it contains. */
  ~walk_tree();

  /* GETTERS, SETTERS, SIMPLE UTILITIES */

  walk_node<Dim> *root() const;

  point<Dim> endpoint() const override;

  bool is_leaf() const;

  /* PRIMITIVE OPERATIONS (see Clisby (2010), Section 2.5) */

  /**
   * @brief Find a node by its id
   *
   * @param n node id
   *
   * @note Runs in constant time.
   *
   * @warning This function can only be used on trees initialized with balanced=true. Moreover,
   * it cannot be used on trees currently being transformed (e.g. via rotation).
   *
   * @return reference to the node
   */
  walk_node<Dim> &find_node(int n);

  /* HIGH-LEVEL FUNCTIONS (see Clisby (2010), Section 2.7) */

  /**
   * @brief Attempt to pivot the walk about the given lattice site using Clisby's Attempt_pivot_simple function.
   *
   * @param n Lattice site to pivot about. Must be greater than 0 and less than the number of lattice sites.
   * @param r Transformation to apply to the walk.
   *
   * @note Since lattice sites are numbered starting at 0, the restriction n > 0 is equivalent to prohibiting
   * gloval transformations of the walk.
   *
   * @return Whether the pivot was successful.
   */
  bool try_pivot(int n, const transform<Dim> &r);

  /**
   * @brief Attempt to pivot the walk about the given lattice site using Clisby's Attempt_pivot_fast function.
   *
   * @param n Lattice site to pivot about. Must be greater than 0 and less than the number of lattice sites.
   * @param r Transformation to apply to the walk.
   *
   * @note Since lattice sites are numbered starting at 0, the restriction n > 0 is equivalent to prohibiting
   * gloval transformations of the walk.
   *
   * @return Whether the pivot was successful.
   */
  bool try_pivot_fast(int n, const transform<Dim> &r);

  /**
   * @brief Attempts to pivot the walk about a randomly chosen lattice site with a random transform.
   *
   * @param fast Whether to use the fast version of the pivot function.
   *
   * @return Whether the pivot was successful.
   */
  bool rand_pivot(bool fast = true) override;

  /* OTHER FUNCTIONS */

  /**
   * @brief Get the sequence of lattice sites that the walk passes through.
   *
   * @return Sequence of lattice sites.
   */
  std::vector<point<Dim>> steps() const;

  /**
   * @brief Check whether the walk is self-avoiding using a naive algorithm.
   *
   * @note This function runs in O(n^2) time, where n is the number of lattice sites
   * and is mostly useful for testing purposes.
   *
   * @return Whether the walk is self-avoiding.
   */
  bool self_avoiding() const override;

  /** @brief Export the walk to a CSV file. */
  void export_csv(const std::string &path) const override;

  /** @brief Export tree to GraphViz format. */
  void todot(const std::string &path) const;

private:
  std::unique_ptr<walk_node<Dim>> root_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> dist_; // distribution for choosing a random lattice site
  walk_node<Dim> *buf_;                     // buffer into which nodes are allocated (used for fast node lookup by id)
};

} // namespace pivot
