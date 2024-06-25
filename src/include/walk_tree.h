#pragma once

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk_node;

template <int Dim> class walk_tree : public walk_base {

public:
  walk_tree(int num_sites, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  walk_tree(const std::string &path, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  walk_tree(const std::vector<point> &steps, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  ~walk_tree();

  walk_node<Dim> *root() const;

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
  walk_node<Dim> &find_node(int n);

  bool try_pivot(int n, const transform &r);

  bool rand_pivot() override;

  std::vector<point> steps() const;

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

private:
  std::unique_ptr<walk_node<Dim>> root_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> dist_;
  walk_node<Dim> *buf_;

  walk_tree(walk_node<Dim> *root);
};

} // namespace pivot
