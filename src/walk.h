#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#include <boost/unordered/unordered_map.hpp>

#include "utils.h"
#include "walk_base.h"

namespace pivot {

class walk : public walk_base {

public:
  walk(int num_steps);

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk() = default;

  int num_steps() const;

  point endpoint() const override;

  std::optional<std::vector<point>> try_pivot(int step, const rot &r) const;

  std::pair<int, std::optional<std::vector<point>>> try_rand_pivot() const;

  bool rand_pivot() override;

  bool rand_pivot(int num_workers);

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

protected:
  std::vector<point> steps_;
  boost::unordered_map<point, int, point_hash> occupied_;

  void do_pivot(int step, std::vector<point> &new_points);

  point pivot_point(int step, int i, rot r) const;
};

} // namespace pivot
