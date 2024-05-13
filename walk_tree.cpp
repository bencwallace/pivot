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

void walk_tree::rotate_left() {
    if (right_ == nullptr) {
        throw std::invalid_argument("right_ must not be null");
    }
    auto temp_tree = right_;

    // update pointers
    set_right(temp_tree->right_);
    temp_tree->right_ = temp_tree->left_;  // temp_tree->set_right(temp_tree->left_) sets parent unnecessarily
    temp_tree->set_left(left_);
    left_ = temp_tree;  // set_left(temp_tree) sets parent unnecessarily

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

void walk_tree::rotate_right() {
    if (left_ == nullptr) {
        throw std::invalid_argument("left_ must not be null");
    }
    auto temp_tree = left_;

    // update pointers
    set_left(temp_tree->left_);
    temp_tree->left_ = temp_tree->right_;  // temp_tree->set_left(temp_tree->right_) sets parent unnecessarily
    temp_tree->set_right(right_);
    right_ = temp_tree;  // set_right(temp_tree) sets parent unnecessarily

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

void walk_tree::set_left(walk_tree *left) {
    left_ = left;
    if (left != nullptr) {
        left->parent_ = this;
    }
}

void walk_tree::set_right(walk_tree *right) {
    right_ = right;
    if (right != nullptr) {
        right->parent_ = this;
    }
}

void walk_tree::merge() {
    int left_sites = left_ == nullptr ? 1 : left_->num_sites_;
    int right_sites = right_ == nullptr ? 1 : right_->num_sites_;
    num_sites_ = left_sites + right_sites;
}

Agnode_t *walk_tree::todot(Agraph_t *g) {
    auto name = std::to_string(id_);
    Agnode_t *node = agnode(g, (char *) name.c_str(), 1);
    agset(node, (char *) "shape", (char *) "box");

    auto label = "id: " + std::to_string(id_) + "\\l";
    label += "num_sites: " + std::to_string(num_sites_) + "\\l";
    label += "symm: " + symm_.to_string();
    agset(node, (char *) "label", (char *) label.c_str());

    if (left_ != nullptr) {
        agedge(g, node, left_->todot(g), nullptr, 1);
    } else {
        auto left_node = agnode(g, (char *) (name + "L").c_str(), 1);
        agset(left_node, (char *) "label", (char *) std::to_string(id_ - 1).c_str());
        agedge(g, node, left_node, nullptr, 1);
    }
    if (right_ != nullptr) {
        agedge(g, node, right_->todot(g), nullptr, 1);
    } else {
        auto right_node = agnode(g, (char *) (name + "R").c_str(), 1);
        agset(right_node, (char *) "label", (char *) std::to_string(id_).c_str());
        agedge(g, node, right_node, nullptr, 1);
    }
    return node;
}

void walk_tree::todot(std::string path) {
    GVC_t *gvc = gvContext();
    Agraph_t *g = agopen((char *) "G", Agdirected, nullptr);
    agattr(g, AGNODE, (char *) "shape", (char *) "circle");

    todot(g);

    gvLayout(gvc, g, "dot");
    gvRenderFilename(gvc, g, "dot", path.c_str());

    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
}

} // namespace pivot
