#pragma once

#include <vector>

#include <gvc.h>

#include "graphviz.h"
#include "utils.h"
#include "walk_base.h"

namespace pivot {

class walk_tree : public walk_base {

public:
  static walk_tree *line(int dim, int num_sites, bool balanced = true);

  static walk_tree *pivot_rep(const std::vector<point> &steps);

  static walk_tree *balanced_rep(const std::vector<point> &steps);

  walk_tree(const walk_tree &w) = delete;
  walk_tree(walk_tree &&w) = delete;
  walk_tree &operator=(const walk_tree &w) = delete;

  ~walk_tree();

  int dim() const;

  bool is_leaf() const;

  point endpoint() const override;

  bool try_pivot(int n, const transform &t);

  bool rand_pivot() override;

  bool self_avoiding() const override;

  std::vector<point> steps() const;

  void export_csv(const std::string &path) const override;

  void todot(const std::string &path) const;

private:
  int id_;
  int num_sites_;
  walk_tree *parent_{};
  walk_tree *left_{};
  walk_tree *right_{};
  transform symm_;
  box bbox_;
  point end_;

  walk_tree(int id, int num_sites, const transform &symm, const box &bbox, const point &end);

  static walk_tree *balanced_rep(std::span<const point> steps, int start);

  void rotate_left();

  void rotate_right();

  void shuffle_up(int id);

  void shuffle_down();

  bool intersect() const;

  friend bool intersect(const walk_tree *l_walk, const walk_tree *r_walk, const point &l_anchor, const point &r_anchor,
                        const transform &l_symm, const transform &r_symm);

  void set_left(walk_tree *left);
  void set_right(walk_tree *right);
  void merge();

  static walk_tree &leaf(int dim);

  Agnode_t *todot(Agraph_t *g, const cgraph_t &cgraph) const;
};

bool intersect(const walk_tree *l_walk, const walk_tree *r_walk, const point &l_anchor, const point &r_anchor,
               const transform &l_symm, const transform &r_symm);

} // namespace pivot
