#pragma once

#include <gvc.h>

#include "utils.h"

namespace pivot {

class walk_tree {

public:
    static walk_tree line(int num_sites);

    static walk_tree pivot_rep(int num_sites, point *steps);

    void todot(std::string path);

private:
    int id_;
    int num_sites_;
    walk_tree *parent_{};
    walk_tree *left_{};
    walk_tree *right_{};
    rot symm_;

    walk_tree(int id, int num_sites, rot symm);

    Agnode_t *todot(Agraph_t *g);

};

} // namespace pivot
