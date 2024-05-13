#include <stdexcept>

#include "utils.h"
#include "walk_tree.h"

namespace pivot {

walk_tree walk_tree::line(int num_sites) {
    if (num_sites < 2) {
        throw std::invalid_argument("num_sites must be at least 2");
    }
    point steps[num_sites];
    for (int i = 0; i < num_sites; ++i) {
        steps[i] = point(i, 0);
    }
    walk_tree root = pivot_rep(num_sites, steps);
    return root;
}

walk_tree walk_tree::pivot_rep(int num_sites, point *steps) {
    if (num_sites < 2) {
        throw std::invalid_argument("num_sites must be at least 2");
    }
    walk_tree root = walk_tree(1, num_sites, rot(steps[0], steps[1]));
    auto node = &root;
    for (int i = 0; i < num_sites - 2; ++i) {
        node->right_ = new walk_tree(i + 2, num_sites - i - 1, rot(steps[i + 1], steps[i + 2]));
        node->right_->parent_ = node;
        node = node->right_;
    }
    return root;
}

walk_tree::walk_tree(int id, int num_sites, rot symm) : id_(id), num_sites_(num_sites), symm_(symm) {}

} // namespace pivot
