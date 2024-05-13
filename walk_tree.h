#pragma once

#include <vector>

#include <gvc.h>

#include "utils.h"

namespace pivot {

class walk_tree {

public:
    static walk_tree *line(int num_sites, bool balanced = true);

    static walk_tree *pivot_rep(int num_sites, point *steps);

    static walk_tree *balanced_rep(int num_sites, point *steps);

    ~walk_tree();

    bool is_leaf() const;

    void rotate_left();

    void rotate_right();

    void shuffle_up(int id);

    void shuffle_down();

    bool intersect() const;

    bool try_pivot(int n, rot r);

    std::vector<point> steps() const;

    void export_csv(std::string path);

    void todot(std::string path);

    static walk_tree *leaf();

    friend bool intersect(
        const walk_tree *l_walk,
        const walk_tree *r_walk,
        const point &l_anchor,
        const point &r_anchor,
        const rot &l_symm,
        const rot &r_symm);

private:
    int id_;
    int num_sites_;
    walk_tree *parent_{};
    walk_tree *left_{};
    walk_tree *right_{};
    rot symm_;
    box bbox_;
    point end_;

    static walk_tree *leaf_;

    walk_tree(int id, int num_sites, rot symm, box bbox, point end);

    static walk_tree *balanced_rep(int num_sites, point *steps, int start);

    void set_left(walk_tree *left);
    void set_right(walk_tree *right);
    void merge();

    Agnode_t *todot(Agraph_t *g);

};

bool intersect(
    const walk_tree *l_walk,
    const walk_tree *r_walk,
    const point &l_anchor,
    const point &r_anchor,
    const rot &l_symm,
    const rot &r_symm);

} // namespace pivot
