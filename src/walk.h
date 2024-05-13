#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include <boost/unordered/unordered_map.hpp>

#include "utils.h"

namespace pivot {

class walk {

public:
  walk(int num_steps);

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk();

  point *try_pivot(int step, const rot &r) const;

  std::pair<int, point *> try_rand_pivot() const;

  bool rand_pivot();

  bool rand_pivot(int num_workers);

  bool self_avoiding() const;

  void export_csv(const std::string &path) const;

protected:
  int num_steps_;
  point *steps_;
  boost::unordered_map<point, int, point_hash> occupied_;

  void do_pivot(int step, point *new_points);

  point pivot_point(int step, int i, rot r) const;
};

} // namespace pivot
