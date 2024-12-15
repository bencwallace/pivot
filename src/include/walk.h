#pragma once

#include <future>
#include <optional>
#include <random>

#include <boost/unordered/unordered_flat_map.hpp>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk : public walk_base<Dim> {

public:
  walk(const std::vector<point<Dim>> &steps, std::optional<unsigned int> seed = std::nullopt);
  walk(int num_steps, std::optional<unsigned int> seed = std::nullopt);
  walk(const std::string &path, std::optional<unsigned int> seed = std::nullopt);

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk() = default;

  point<Dim> operator[](int i) const { return steps_[i]; }

  int num_steps() const { return steps_.size(); }

  point<Dim> endpoint() const override { return steps_.back(); }

  std::optional<std::vector<point<Dim>>> try_pivot(int step, const transform<Dim> &trans) const;

  std::pair<int, std::optional<std::vector<point<Dim>>>> try_rand_pivot() const;

  bool rand_pivot(bool fast = false) override;

  bool rand_pivot(int num_workers);

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

protected:
  std::vector<point<Dim>> steps_;
  boost::unordered_flat_map<point<Dim>, int, point_hash> occupied_;

  mutable std::mt19937 rng_;
  mutable std::uniform_int_distribution<int> dist_;

  void do_pivot(int step, std::vector<point<Dim>> &new_points);

  point<Dim> pivot_point(int step, int i, const transform<Dim> &trans) const;
};

} // namespace pivot
