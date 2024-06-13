#pragma once

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk_node;

template <int Dim> class walk_tree : public walk_base<Dim> {

public:
  walk_tree(int num_sites, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  walk_tree(const std::string &path, std::optional<unsigned int> seed = std::nullopt, bool balanced = true);

  ~walk_tree();

  point<Dim> endpoint() const override;

  bool is_leaf() const;

  bool try_pivot(int n, const transform<Dim> &r);

  bool rand_pivot() override;

  std::vector<point<Dim>> steps() const;

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

private:
  std::unique_ptr<walk_node<Dim>> root_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> dist_;

  walk_tree(walk_node<Dim> *root);
};

} // namespace pivot
