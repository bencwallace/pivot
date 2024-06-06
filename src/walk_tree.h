#pragma once

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "utils.h"
#include "walk_base.h"
#include "walk_node.h"

namespace pivot {

template <int Dim> class walk_tree : public walk_base<Dim> {

public:
  walk_tree(int num_sites, std::optional<unsigned int> seed = std::nullopt, bool balanced = true) {
    if (num_sites < 2) {
      throw std::invalid_argument("num_sites must be at least 2");
    }
    std::vector<point<Dim>> steps(num_sites);
    for (int i = 0; i < num_sites; ++i) {
      steps[i] = (i + 1) * point<Dim>::unit(0);
    }
    root_ = balanced ? std::unique_ptr<walk_node<Dim>>(walk_node<Dim>::balanced_rep(steps))
                     : std::unique_ptr<walk_node<Dim>>(walk_node<Dim>::pivot_rep(steps));

    rng_ = std::mt19937(seed.value_or(std::random_device()()));
    dist_ = std::uniform_int_distribution<int>(1, num_sites - 1);
  }

  point<Dim> endpoint() const override { return root_->endpoint(); }

  bool is_leaf() const { return root_->is_leaf(); }

  bool try_pivot(int n, const transform<Dim> &r) {
    root_->shuffle_up(n);
    root_->symm_ = root_->symm_ * r; // modify in-place
    auto success = !root_->intersect();
    if (!success) {
      root_->symm_ = root_->symm_ * r.inverse(); // TODO: backup symm_
    } else {
      root_->merge();
    }
    root_->shuffle_down();
    return success;
  }

  bool rand_pivot() override {
    auto site = dist_(rng_);
    auto r = transform<Dim>::rand(rng_);
    return try_pivot(site, r);
  }

  std::vector<point<Dim>> steps() const { return root_->steps(); }

  bool self_avoiding() const override {
    auto steps = this->steps();
    for (size_t i = 0; i < steps.size(); ++i) {
      for (size_t j = i + 1; j < steps.size(); ++j) {
        if (steps[i] == steps[j]) {
          return false;
        }
      }
    }
    return true;
  }

  void export_csv(const std::string &path) const override { return to_csv(path, steps()); }

private:
  std::unique_ptr<walk_node<Dim>> root_;
  std::mt19937 rng_;
  std::uniform_int_distribution<int> dist_;

  walk_tree(walk_node<Dim> *root) : root_(root) {}
};

} // namespace pivot
