#include "walk_node.h"

namespace pivot {

/* PRIMITIVE OPERATIONS */

// Note: A detail missing from Clisby's paper regarding tree rotations is that parent pointers must be updated,
// except when the rotation is called from shuffle_intersect.

template <int Dim> walk_node<Dim> *walk_node<Dim>::rotate_left(bool set_parent) {
  if (right_->is_leaf()) {
    throw std::invalid_argument("can't rotate left on a leaf node");
  }
  auto temp_tree = right_;

  // update pointers
  right_ = temp_tree->right_;
  if (set_parent && temp_tree->right_ != nullptr) {
    temp_tree->right_->parent_ = this;
  }
  temp_tree->right_ = temp_tree->left_; // temp_tree->set_right(temp_tree->left_) sets parent unnecessarily
  temp_tree->left_ = left_;
  if (set_parent && left_ != nullptr) {
    left_->parent_ = temp_tree;
  }
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

  return this;
}

template <int Dim> walk_node<Dim> *walk_node<Dim>::rotate_right(bool set_parent) {
  if (left_->is_leaf()) {
    throw std::invalid_argument("can't rotate right on a leaf node");
  }
  auto temp_tree = left_;

  // update pointers
  left_ = temp_tree->left_;
  if (set_parent && temp_tree->left_ != nullptr) {
    temp_tree->left_->parent_ = this;
  }
  temp_tree->left_ = temp_tree->right_; // temp_tree->set_left(temp_tree->right_) sets parent unnecessarily
  temp_tree->right_ = right_;
  if (set_parent && right_ != nullptr) {
    right_->parent_ = temp_tree;
  }
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

  return this;
}

template <int Dim> void walk_node<Dim>::merge() {
  num_sites_ = left_->num_sites_ + right_->num_sites_;

  bbox_ = left_->bbox_ | (left_->end_ + symm_ * right_->bbox_);
  end_ = left_->end_ + symm_ * right_->end_;
}

/* USER LEVEL OPERATIONS */

template <int Dim> walk_node<Dim> *walk_node<Dim>::shuffle_up(int id) {
  if (id < left_->num_sites_) {
    left_->shuffle_up(id);
    rotate_right();
  } else if (id > left_->num_sites_) {
    right_->shuffle_up(id - left_->num_sites_);
    rotate_left();
  }

  return this;
}

template <int Dim> walk_node<Dim> *walk_node<Dim>::shuffle_down() {
  int id = std::floor((num_sites_ + 1) / 2.0);
  if (id < left_->num_sites_) {
    rotate_right();
    right_->shuffle_down();
  } else if (id > left_->num_sites_) {
    rotate_left();
    left_->shuffle_down();
  }

  return this;
}

template <int Dim> bool walk_node<Dim>::intersect() const {
  return ::pivot::intersect<Dim>(left_, right_, point<Dim>(), left_->end_, transform<Dim>(), symm_);
}

template <int Dim>
bool intersect(const walk_node<Dim> *l_walk, const walk_node<Dim> *r_walk, const point<Dim> &l_anchor,
               const point<Dim> &r_anchor, const transform<Dim> &l_symm, const transform<Dim> &r_symm) {
  auto l_box = l_anchor + l_symm * l_walk->bbox_;
  auto r_box = r_anchor + r_symm * r_walk->bbox_;
  if ((l_box & r_box).empty()) {
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

template <int Dim> bool walk_node<Dim>::shuffle_intersect(const transform<Dim> &t, std::optional<bool> is_left_child) {
  return shuffle_intersect(t, std::nullopt, is_left_child);
}

template <int Dim>
bool walk_node<Dim>::shuffle_intersect(const transform<Dim> &t, std::optional<bool> was_left_child,
                                       std::optional<bool> is_left_child) {
  /* BASE CASE */
  if (was_left_child.has_value()) {
    if (was_left_child.value()) {
      if (::pivot::intersect(left_, right_->right_, point<Dim>(), left_->end_ + symm_ * t * right_->left_->end_,
                             transform<Dim>(), symm_ * t * right_->symm_)) {
        return true;
      }
    } else {
      if (::pivot::intersect(left_->left_, right_, point<Dim>(), left_->end_, transform<Dim>(), symm_ * t)) {
        return true;
      }
    }
  } else {
    if (::pivot::intersect(left_, right_, point<Dim>(), left_->end_, transform<Dim>(), symm_ * t)) {
      return true;
    }
  }

  if (parent_ == nullptr) {
    return false;
  }

  /* RECURSION */
  auto is_left_child_new = parent_->is_left_child();

  // Note: A detail left out in Clisby's paper is that the left or right sub-node must also be copied
  // and the copied parent node must be have its left and right children updated. This is to avoid
  // modifying the original tree when performing a right or left rotation.
  walk_node w(*parent_);
  bool result = false;
  if (is_left_child.value()) {
    auto w1 = parent_->right_->is_leaf() ? leaf() : walk_node(*parent_->right_);
    w.set_left(this);
    w.set_right(&w1);
    w.rotate_right(false);
    result = w.shuffle_intersect(t, is_left_child, is_left_child_new);
  } else {
    auto w1 = parent_->left_->is_leaf() ? leaf() : walk_node(*parent_->left_);
    w.set_left(&w1);
    w.set_right(this);
    w.rotate_left(false);
    result = w.shuffle_intersect(t, is_left_child, is_left_child_new);
  }
  return result;
}

} // namespace pivot
