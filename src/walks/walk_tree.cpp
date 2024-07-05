#include <cstdlib>
#include <new>
#include <stack>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "defines.h"
#include "utils.h"
#include "walk_node.h"
#include "walk_tree.h"

namespace pivot {

template <int Dim>
walk_tree<Dim>::walk_tree(int num_sites, std::optional<unsigned int> seed, bool balanced)
    : walk_tree(line<Dim>(num_sites), seed, balanced) {}

template <int Dim>
walk_tree<Dim>::walk_tree(const std::string &path, std::optional<unsigned int> seed, bool balanced)
    : walk_tree(from_csv<Dim>(path), seed, balanced) {}

template <int Dim>
walk_tree<Dim>::walk_tree(const std::vector<point<Dim>> &steps, std::optional<unsigned int> seed, bool balanced) {
  if (steps.size() < 2) {
    throw std::invalid_argument("walk must have at least 2 sites (1 step)");
  }
  buf_ = nullptr;
  if (balanced) {
    size_t buf_size = sizeof(walk_node<Dim>) * (steps.size() - 1);
    constexpr auto alignment = std::align_val_t(alignof(walk_node<Dim>));
    buf_ = static_cast<walk_node<Dim> *>(::operator new[](buf_size, alignment));
  }
  root_ = balanced ? std::unique_ptr<walk_node<Dim>>(walk_node<Dim>::balanced_rep(steps, buf_))
                   : std::unique_ptr<walk_node<Dim>>(walk_node<Dim>::pivot_rep(steps, buf_));

  rng_ = std::mt19937(seed.value_or(std::random_device()()));
  dist_ = std::uniform_int_distribution<int>(1, steps.size() - 1);
}

template <int Dim> walk_tree<Dim>::~walk_tree() {
  std::stack<walk_node<Dim> *> nodes;
  nodes.push(root_.release());
  while (!nodes.empty()) {
    auto curr = nodes.top();
    nodes.pop();
    if (curr->left_ && !curr->left_->is_leaf()) {
      nodes.push(curr->left_);
    }
    if (curr->right_ && !curr->right_->is_leaf()) {
      nodes.push(curr->right_);
    }
    curr->~walk_node();
  }

  if (buf_) {
    ::operator delete[](buf_, std::align_val_t(alignof(walk_node<Dim>)));
  }
}

template <int Dim> walk_tree<Dim>::walk_tree(walk_node<Dim> *root) : root_(root) {}

template <int Dim> walk_node<Dim> *walk_tree<Dim>::root() const { return root_.get(); }

template <int Dim> point<Dim> walk_tree<Dim>::endpoint() const { return root_->endpoint(); }

template <int Dim> bool walk_tree<Dim>::is_leaf() const { return root_->is_leaf(); }

template <int Dim> std::vector<point<Dim>> walk_tree<Dim>::steps() const { return root_->steps(); }

template <int Dim> walk_node<Dim> &walk_tree<Dim>::find_node(int n) {
  if (!buf_) {
    throw std::runtime_error("find_node can only be used on trees initialized with balanced=true");
  }
  walk_node<Dim> &result = buf_[n - 1];
  assert(result.id_ == n);
  return result;
}

template <int Dim> void walk_tree<Dim>::shuffle_up(int id) {
  walk_node<Dim> *node = &find_node(id);
  walk_node<Dim> *parent = node->parent_;
  while (parent) {
    if (parent->left_ == node) {
      parent->rotate_right();
    } else {
      parent->rotate_left();
    }
    node = parent;
    parent = node->parent_;
  }
}

template <int Dim> void walk_tree<Dim>::shuffle_down() {
  root_->shuffle_down();
}

template <int Dim> bool walk_tree<Dim>::try_pivot(int n, const transform<Dim> &r) {
  shuffle_up(n);
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

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, WALK_TREE_INST, ~)

} // namespace pivot
