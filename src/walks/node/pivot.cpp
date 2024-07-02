#include "walk_node.h"

namespace pivot {

walk_node *walk_node::rotate_left() {
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

  return this;
}

walk_node *walk_node::rotate_right() {
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

  return this;
}

walk_node *walk_node::shuffle_up(int id) {
  if (id < left_->num_sites_) {
    left_->shuffle_up(id);
    rotate_right();
  } else if (id > left_->num_sites_) {
    right_->shuffle_up(id - left_->num_sites_);
    rotate_left();
  }

  return this;
}

walk_node *walk_node::shuffle_down() {
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

bool intersect(const walk_node *l_walk, const walk_node *r_walk, const point &l_anchor, const point &r_anchor,
               const transform &l_symm, const transform &r_symm) {
  static box l_box(l_walk->bbox_.dim_);
  static box r_box(r_walk->bbox_.dim_);

  l_box = l_symm * l_walk->bbox_;
  l_box += l_anchor;

  r_box = r_symm * r_walk->bbox_;
  r_box += r_anchor;
  r_box &= l_box;

  if (r_box.empty()) {
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

void walk_node::merge() {
  num_sites_ = left_->num_sites_;
  num_sites_ += right_->num_sites_;

  bbox_ = symm_ * right_->bbox_;
  bbox_ += left_->end_;
  bbox_ |= left_->bbox_;

  end_ = symm_ * right_->end_;
  end_ += left_->end_;
}

} // namespace pivot
