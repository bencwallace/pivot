#pragma once

#include <gvc.h>

#include "utils.h"

namespace pivot {

class walk_tree {

public:
    static walk_tree *line(int num_sites, bool balanced = true);

    static walk_tree *pivot_rep(int num_sites, point *steps);

    static walk_tree *balanced_rep(int num_sites, point *steps);

    ~walk_tree();

    void rotate_left();

    void rotate_right();

    void shuffle_up(int id);

    void shuffle_down();

    void todot(std::string path);

private:
    int id_;
    int num_sites_;
    walk_tree *parent_{};
    walk_tree *left_{};
    walk_tree *right_{};
    rot symm_;
    box bbox_;
    point end_;

    walk_tree(int id, int num_sites, rot symm, box bbox, point end);

    static walk_tree *balanced_rep(int num_sites, point *steps, int start);

    void set_left(walk_tree *left);
    void set_right(walk_tree *right);
    void merge();

    Agnode_t *todot(Agraph_t *g);

};

} // namespace pivot
