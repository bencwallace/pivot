#pragma once

#include <vector>

#include <gvc.h>

#include "graphviz.h"
#include "utils.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk_node;

template <int Dim> class walk_tree;

template <int Dim>
bool intersect(const walk_node<Dim> *l_walk, const walk_node<Dim> *r_walk, const point<Dim> &l_anchor,
               const point<Dim> &r_anchor, const transform<Dim> &l_symm, const transform<Dim> &r_symm);

template <int Dim> class walk_node {

public:
  static walk_node *pivot_rep(const std::vector<point<Dim>> &steps) {
    int num_sites = steps.size();
    if (num_sites < 2) {
      throw std::invalid_argument("num_sites must be at least 2");
    }
    walk_node *root = new walk_node(1, num_sites, transform(steps[0], steps[1]), box<Dim>(steps), steps[num_sites - 1]);
    auto node = root;
    for (int i = 0; i < num_sites - 2; ++i) {
      node->right_ = new walk_node(i + 2, num_sites - i - 1, transform(steps[i + 1], steps[i + 2]),
                                   box(std::span<const point<Dim>>(steps).subspan(i + 1)),
                                   steps[num_sites - 1]); // TODO: double-check this
      node->right_->parent_ = node;
      node = node->right_;
    }
    return root;
  }

  static walk_node *balanced_rep(const std::vector<point<Dim>> &steps) { return balanced_rep(steps, 1); }

  walk_node(const walk_node &w) = delete;
  walk_node(walk_node &&w) = delete;
  walk_node &operator=(const walk_node &w) = delete;

  ~walk_node() {
    if (left_ != nullptr && left_ != &leaf()) {
      delete left_;
    }
    if (right_ != nullptr && right_ != &leaf()) {
      delete right_;
    }
  }

  bool is_leaf() const { return left_ == nullptr && right_ == nullptr; }

  point<Dim> endpoint() const { return end_; }

  std::vector<point<Dim>> steps() const {
    std::vector<point<Dim>> result;
    if (is_leaf()) {
      result.push_back(end_);
      return result;
    }

    auto left_steps = left_->steps();
    result.insert(result.begin(), left_steps.begin(), left_steps.end());

    auto right_steps = right_->steps();
    for (auto &step : right_steps) {
      result.push_back(left_->end_ + symm_ * step);
    }

    return result;
  }

  void todot(const std::string &path) const {
    gvc_t &gvc = gvc_t::load();
    cgraph_t &cgraph = cgraph_t::load();

    GVC_t *context = gvc.gvContext();
    Agraph_t *g = cgraph.agopen((char *)"G", cgraph.Agdirected, nullptr);
    cgraph.agattr(g, AGNODE, (char *)"shape", (char *)"circle");

    todot(g, cgraph);

    gvc.gvLayout(context, g, "dot");
    gvc.gvRenderFilename(context, g, "dot", path.c_str());

    gvc.gvFreeLayout(context, g);
    cgraph.agclose(g);
    gvc.gvFreeContext(context);
  }

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

  walk_node(int id, int num_sites, const transform<Dim> &symm, const box<Dim> &bbox, const point<Dim> &end)
      : id_(id), num_sites_(num_sites), symm_(symm), bbox_(bbox), end_(end) {}

  static walk_node *balanced_rep(std::span<const point<Dim>> steps, int start) {
    int num_sites = steps.size();
    if (num_sites < 1) {
      throw std::invalid_argument("num_sites must be at least 1");
    }
    if (num_sites == 1) {
      return &leaf();
    }
    int n = std::floor((1 + num_sites) / 2.0);
    walk_node *root = new walk_node(start + n - 1, num_sites, transform(steps[n - 1], steps[n]), box(steps),
                                    steps[num_sites - 1] - steps[0] + point<Dim>::unit(0));
    if (n >= 1) {
      root->left_ = balanced_rep(steps.subspan(0, n), start);
      root->left_->parent_ = root;
    }
    if (num_sites - n >= 1) {
      root->right_ = balanced_rep(steps.subspan(n), start + n);
      root->right_->parent_ = root;
    }
    return root;
  }

  void rotate_left() {
    if (right_->is_leaf()) {
      throw std::invalid_argument("can't rotate left on a leaf node");
    }
    auto temp_tree = right_;

    // update pointers
    set_right(temp_tree->right_);
    temp_tree->right_ = temp_tree->left_; // temp_tree->set_right(temp_tree->left_) sets parent unnecessarily
    temp_tree->set_left(left_);
    left_ = temp_tree; // set_left(temp_tree) sets parent unnecessarily

    // update symmetries
    auto temp_symm = symm_;
    symm_ = temp_symm * left_->symm_;
    left_->symm_ = temp_symm;

    // merge
    left_->merge();

    // update IDs
    int temp_id = id_;
    id_ = left_->id_;
    left_->id_ = temp_id;
  }

  void rotate_right() {
    if (left_->is_leaf()) {
      throw std::invalid_argument("can't rotate right on a leaf node");
    }
    auto temp_tree = left_;

    // update pointers
    set_left(temp_tree->left_);
    temp_tree->left_ = temp_tree->right_; // temp_tree->set_left(temp_tree->right_) sets parent unnecessarily
    temp_tree->set_right(right_);
    right_ = temp_tree; // set_right(temp_tree) sets parent unnecessarily

    // update symmetries
    auto temp_symm = symm_;
    symm_ = right_->symm_;
    right_->symm_ = symm_.inverse() * temp_symm;

    // merge
    right_->merge();

    // update IDs
    int temp_id = id_;
    id_ = right_->id_;
    right_->id_ = temp_id;
  }

  void shuffle_up(int id) {
    if (id < left_->num_sites_) {
      left_->shuffle_up(id);
      rotate_right();
    } else if (id > left_->num_sites_) {
      right_->shuffle_up(id - left_->num_sites_);
      rotate_left();
    }
  }

  void shuffle_down() {
    int id = std::floor((num_sites_ + 1) / 2.0);
    if (id < left_->num_sites_) {
      rotate_right();
      right_->shuffle_down();
    } else if (id > left_->num_sites_) {
      rotate_left();
      left_->shuffle_down();
    }
  }

  bool intersect() const {
    return ::pivot::intersect<Dim>(left_, right_, point<Dim>(), left_->end_, transform<Dim>(), symm_);
  }

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

  void merge() {
    num_sites_ = left_->num_sites_ + right_->num_sites_;

    bbox_ = left_->bbox_ + (left_->end_ + symm_ * right_->bbox_);
    end_ = left_->end_ + symm_ * right_->end_;
  }

  static walk_node &leaf() {
    std::array<interval, Dim> intervals;
    intervals[0] = interval(1, 1);
    for (int i = 1; i < Dim; ++i) {
      intervals[i] = interval(0, 0);
    }
    static auto leaf_ = walk_node(0, 1, transform<Dim>(), box<Dim>(intervals), point<Dim>::unit(0));
    return leaf_;
  }

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const {
    auto name = std::to_string(id_);
    Agnode_t *node = cgraph.agnode(g, (char *)name.c_str(), 1);
    if (cgraph.agset((void *)node, (char *)"shape", (char *)"box") != 0) {
      throw std::runtime_error("failed to set shape");
    }

    auto label = "id: " + std::to_string(id_) + "\\l";
    label += "num_sites: " + std::to_string(num_sites_) + "\\l";
    label += "symm: " + symm_.to_string() + "\\l";
    label += "box: " + bbox_.to_string() + "\\l";
    label += "end: " + end_.to_string() + "\\l";
    cgraph.agset(node, (char *)"label", (char *)label.c_str());

    if (left_ != nullptr) {
      Agnode_t *left_node;
      if (left_->is_leaf()) {
        left_node = cgraph.agnode(g, (char *)(name + "L").c_str(), 1);
        cgraph.agset(left_node, (char *)"label", (char *)std::to_string(id_ - 1).c_str());
      } else {
        left_node = left_->todot(g, cgraph);
      }
      cgraph.agedge(g, node, left_node, nullptr, 1);
    }
    if (right_ != nullptr) {
      Agnode_t *right_node;
      if (right_->is_leaf()) {
        right_node = cgraph.agnode(g, (char *)(name + "R").c_str(), 1);
        cgraph.agset(right_node, (char *)"label", (char *)std::to_string(id_).c_str());
      } else {
        right_node = right_->todot(g, cgraph);
      }
      cgraph.agedge(g, node, right_node, nullptr, 1);
    }
    return node;
  }

  template <int D>
  friend bool intersect(const walk_node<D> *l_walk, const walk_node<D> *r_walk, const point<D> &l_anchor,
                        const point<D> &r_anchor, const transform<D> &l_symm, const transform<D> &r_symm);
};

template <int Dim>
bool intersect(const walk_node<Dim> *l_walk, const walk_node<Dim> *r_walk, const point<Dim> &l_anchor,
               const point<Dim> &r_anchor, const transform<Dim> &l_symm, const transform<Dim> &r_symm) {
  auto l_box = l_anchor + l_symm * l_walk->bbox_;
  auto r_box = r_anchor + r_symm * r_walk->bbox_;
  if ((l_box * r_box).empty()) {
    return false;
  }

  if (l_walk->num_sites_ <= 2 && r_walk->num_sites_ <= 2) {
    return true;
  }

  if (l_walk->num_sites_ >= r_walk->num_sites_) {
    return intersect(l_walk->right_, r_walk, l_anchor + l_symm * l_walk->left_->end_, r_anchor, l_symm * l_walk->symm_,
                     r_symm) ||
           intersect(l_walk->left_, r_walk, l_anchor, r_anchor, l_symm, r_symm);
  } else {
    return intersect(l_walk, r_walk->left_, l_anchor, r_anchor, l_symm, r_symm) ||
           intersect(l_walk, r_walk->right_, l_anchor, r_anchor + r_symm * r_walk->left_->end_, l_symm,
                     r_symm * r_walk->symm_);
  }
}

template <int Dim> class walk_tree : public walk_base<Dim> {

public:
  walk_tree(int num_sites, bool balanced = true) {
    if (num_sites < 2) {
      throw std::invalid_argument("num_sites must be at least 2");
    }
    std::vector<point<Dim>> steps(num_sites);
    for (int i = 0; i < num_sites; ++i) {
      steps[i] = (i + 1) * point<Dim>::unit(0);
    }
    root_ = balanced ? walk_node<Dim>::balanced_rep(steps) : walk_node<Dim>::pivot_rep(steps);
  }

  ~walk_tree() override { delete root_; }

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
    auto site = 1 + (std::rand() % (root_->num_sites_ - 1)); // NOLINT(clang-analyzer-core.DivideZero)
    auto r = transform<Dim>::rand();
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
  walk_node<Dim> *root_;

  walk_tree(walk_node<Dim> *root) : root_(root) {}
};

} // namespace pivot
