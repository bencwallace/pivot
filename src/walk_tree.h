#pragma once

#include <vector>

#include <gvc.h>

#include "graphviz.h"
#include "utils.h"
#include "walk_base.h"

namespace pivot {

class walk_tree : public walk_base {

public:
  static walk_tree *line(int num_sites, bool balanced = true);

  static walk_tree *pivot_rep(int num_sites, point *steps);

  static walk_tree *balanced_rep(int num_sites, point *steps);

  walk_tree(const walk_tree &w) = delete;
  walk_tree(walk_tree &&w) = delete;
  walk_tree &operator=(const walk_tree &w) = delete;

  ~walk_tree();

  bool is_leaf() const;

  point endpoint() const override;

  void rotate_left();

  void rotate_right();

  void shuffle_up(int id);

  void shuffle_down();

  bool intersect() const;

  bool try_pivot(int n, const rot &r);

  bool rand_pivot() override;

  bool self_avoiding() const override;

  std::vector<point> steps() const;

  void export_csv(const std::string &path) const override;

  void todot(const std::string &path) const;

  static walk_tree &leaf();

  friend bool intersect(const walk_tree *l_walk, const walk_tree *r_walk, const point &l_anchor, const point &r_anchor,
                        const rot &l_symm, const rot &r_symm);

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

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const;
};

bool intersect(const walk_tree *l_walk, const walk_tree *r_walk, const point &l_anchor, const point &r_anchor,
               const rot &l_symm, const rot &r_symm);

} // namespace pivot
