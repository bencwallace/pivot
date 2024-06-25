#pragma once

#include <future>
#include <optional>
#include <random>

#include <boost/unordered/unordered_map.hpp>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

class walk : public walk_base {

public:
  walk(int dim, int num_steps, std::optional<unsigned int> seed = std::nullopt);

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk() = default;

  point operator[](int i) const { return steps_[i]; }

  int num_steps() const { return steps_.size(); }

  point endpoint() const override { return steps_.back(); }

  std::optional<std::vector<point>> try_pivot(int step, const transform &trans) const;

  std::pair<int, std::optional<std::vector<point>>> try_rand_pivot() const;

  bool rand_pivot() override;

  bool rand_pivot(int num_workers);

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

protected:
  int dim_;
  std::vector<point> steps_;
  boost::unordered_map<point, int, point_hash> occupied_;

  mutable std::mt19937 rng_;
  mutable std::uniform_int_distribution<int> dist_;

  void do_pivot(int step, std::vector<point> &new_points);

  point pivot_point(int step, int i, const transform &trans) const;
};

} // namespace pivot
