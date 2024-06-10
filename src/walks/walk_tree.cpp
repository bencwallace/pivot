#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "walk_tree.h"

namespace pivot {

template <int Dim> walk_tree<Dim>::walk_tree(int num_sites, std::optional<unsigned int> seed, bool balanced) {
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

template <int Dim> walk_tree<Dim>::walk_tree(walk_node<Dim> *root) : root_(root) {}

template <int Dim> point<Dim> walk_tree<Dim>::endpoint() const { return root_->endpoint(); }

template <int Dim> bool walk_tree<Dim>::is_leaf() const { return root_->is_leaf(); }

template <int Dim> std::vector<point<Dim>> walk_tree<Dim>::steps() const { return root_->steps(); }

template <int Dim> bool walk_tree<Dim>::try_pivot(int n, const transform<Dim> &r) {
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

template <int Dim> bool walk_tree<Dim>::rand_pivot() {
  auto site = dist_(rng_);
  auto r = transform<Dim>::rand(rng_);
  return try_pivot(site, r);
}

template <int Dim> bool walk_tree<Dim>::self_avoiding() const {
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

template <int Dim> void walk_tree<Dim>::export_csv(const std::string &path) const { return to_csv(path, steps()); }

#define WALK_TREE_INST(z, n, data) template class walk_tree<n>;
BOOST_PP_REPEAT_FROM_TO(1, 6, WALK_TREE_INST, ~)

} // namespace pivot
