#include "walk_node.h"

namespace pivot {

template <int Dim>
walk_node<Dim>::walk_node(int id, int num_sites, const transform<Dim> &symm, const box<Dim> &bbox,
                          const point<Dim> &end)
    : id_(id), num_sites_(num_sites), symm_(symm), bbox_(bbox), end_(end) {}

template <int Dim> walk_node<Dim> *walk_node<Dim>::pivot_rep(const std::vector<point<Dim>> &steps, walk_node *buf) {
  int num_sites = steps.size();
  if (num_sites < 2) {
    throw std::invalid_argument("num_sites must be at least 2");
  }
  walk_node<Dim> *root =
      buf ? new (buf) walk_node(1, num_sites, transform(steps[0], steps[1]), box<Dim>(steps), steps[num_sites - 1])
          : new walk_node(1, num_sites, transform(steps[0], steps[1]), box<Dim>(steps), steps[num_sites - 1]);
  auto node = root;
  for (int i = 0; i < num_sites - 2; ++i) {
    auto id = i + 2;
    node->right_ = buf ? new (buf + id - 1)
                             walk_node(id, num_sites - i - 1, transform(steps[i + 1], steps[i + 2]),
                                       box(std::span<const point<Dim>>(steps).subspan(i + 1)), steps[num_sites - 1])
                       : new walk_node(i + 2, num_sites - i - 1, transform(steps[i + 1], steps[i + 2]),
                                       box(std::span<const point<Dim>>(steps).subspan(i + 1)),
                                       steps[num_sites - 1]); // TODO: double-check this
    node->right_->parent_ = node;
    node = node->right_;
  }
  return root;
}

template <int Dim>
walk_node<Dim> *walk_node<Dim>::balanced_rep(const std::vector<point<Dim>> &steps, walk_node<Dim> *buf) {
  return balanced_rep(steps, 1, transform<Dim>(), buf);
}

template <int Dim>
walk_node<Dim> *walk_node<Dim>::balanced_rep(std::span<const point<Dim>> steps, int start,
                                             const transform<Dim> &glob_symm, walk_node<Dim> *buf) {
  int num_sites = steps.size();
  if (num_sites < 1) {
    throw std::invalid_argument("num_sites must be at least 1");
  }
  if (num_sites == 1) {
    return &leaf();
  }

  /* The steps span gives an "absolute" view of the walk, but a "relative" view is required, since each sub-tree,
  including the current one, must itself be a walk anchored at the first coordinate vector. The "global symmetry"
  glob_symm represents the transformation "accumulated" since the root of the tree under construction. Its effect
  must be reversed in order to obtain the relative properties of the current node. */
  int n = std::floor((1 + num_sites) / 2.0);
  auto abs_symm = transform(steps[n - 1], steps[n]);
  auto glob_inv = glob_symm.inverse();
  auto rel_symm = glob_inv * abs_symm;
  auto rel_end = glob_inv * (steps.back() - steps.front()) + point<Dim>::unit(0);
  auto rel_box = point<Dim>::unit(0) + glob_inv * (box(steps) - point<Dim>::unit(0));
  int id = start + n - 1;
  walk_node *root = buf ? new (buf + id - 1) walk_node(id, num_sites, rel_symm, rel_box, rel_end)
                        : new walk_node(id, num_sites, rel_symm, rel_box, rel_end);

  if (n >= 1) {
    root->left_ = balanced_rep(steps.subspan(0, n), start, glob_symm, buf);
    root->left_->parent_ = root;
  }
  if (num_sites - n >= 1) {
    root->right_ = balanced_rep(steps.subspan(n), start + n, glob_symm * rel_symm, buf);
    root->right_->parent_ = root;
  }
  return root;
}

template <int Dim> walk_node<Dim> walk_node<Dim>::create_leaf() {
  std::array<interval, Dim> intervals;
  intervals[0] = interval(1, 1);
  for (int i = 1; i < Dim; ++i) {
    intervals[i] = interval(0, 0);
  }
  return walk_node(0, 1, transform<Dim>(), box<Dim>(intervals), point<Dim>::unit(0));
}

template <int Dim> walk_node<Dim> &walk_node<Dim>::leaf() {
  static walk_node<Dim> leaf = create_leaf();
  return leaf;
}

template <int Dim> walk_node<Dim>::~walk_node() = default;

} // namespace pivot
