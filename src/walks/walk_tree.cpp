#include <cstdlib>
#include <new>
#include <stack>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "defines.h"
#include "utils.h"
#include "walk_node.h"
#include "walk_tree.h"

namespace pivot {

walk_tree::walk_tree(int dim, int num_sites, std::optional<unsigned int> seed, bool balanced)
    : walk_tree(line(dim, num_sites), seed, balanced) {}

walk_tree::walk_tree(int dim, const std::string &path, std::optional<unsigned int> seed, bool balanced)
    : walk_tree(from_csv(dim, path), seed, balanced) {}

walk_tree::walk_tree(const std::vector<point> &steps, std::optional<unsigned int> seed, bool balanced)
    : dim_(steps[0].dim()) {
  if (steps.size() < 2) {
    throw std::invalid_argument("walk must have at least 2 sites (1 step)");
  }
  buf_ = nullptr;
  if (balanced) {
    size_t buf_size = sizeof(walk_node) * (steps.size() - 1);
    constexpr auto alignment = std::align_val_t(alignof(walk_node));
    buf_ = static_cast<walk_node *>(::operator new[](buf_size, alignment));
  }
  root_ = balanced ? std::unique_ptr<walk_node>(walk_node::balanced_rep(steps, buf_))
                   : std::unique_ptr<walk_node>(walk_node::pivot_rep(steps, buf_));

  rng_ = std::mt19937(seed.value_or(std::random_device()()));
  dist_ = std::uniform_int_distribution<int>(1, steps.size() - 1);
}

walk_tree::~walk_tree() {
  std::stack<walk_node *> nodes;
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
    ::operator delete[](buf_, std::align_val_t(alignof(walk_node)));
  }
}

walk_node *walk_tree::root() const { return root_.get(); }

point walk_tree::endpoint() const { return root_->endpoint(); }

bool walk_tree::is_leaf() const { return root_->is_leaf(); }

std::vector<point> walk_tree::steps() const { return root_->steps(); }

walk_node &walk_tree::find_node(int n) {
  if (!buf_) {
    throw std::runtime_error("find_node can only be used on trees initialized with balanced=true");
  }
  walk_node &result = buf_[n - 1];
  assert(result.id_ == n);
  return result;
}

bool walk_tree::try_pivot(int n, const transform &r) {
  root_->shuffle_up(n);
  root_->symm_ = root_->symm_ * r; // modify in-place
  auto success = !intersect();
  if (!success) {
    root_->symm_ = root_->symm_ * r.inverse(); // TODO: backup symm_
  } else {
    root_->merge();
  }
  root_->shuffle_down();
  return success;
}

bool walk_tree::rand_pivot() {
  auto site = dist_(rng_);
  auto r = transform::rand(dim_, rng_);
  return try_pivot(site, r);
}

bool walk_tree::self_avoiding() const {
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

void walk_tree::export_csv(const std::string &path) const { return to_csv(path, steps()); }

bool walk_tree::intersect() const {
  return ::pivot::intersect(root_->left_, root_->right_, point(dim_), root_->left_->end_, transform(dim_),
                            root_->symm());
}

} // namespace pivot
