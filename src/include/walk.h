#pragma once

#include <future>
#include <optional>
#include <random>

#include <boost/unordered/unordered_flat_map.hpp>

#include "lattice.h"
#include "walk_base.h"

namespace pivot {

template <int Dim, bool Simd = false> class walk : public walk_base<Dim, Simd> {

public:
  walk(const std::vector<point<Dim, Simd>> &steps, std::optional<unsigned int> seed = std::nullopt);
  walk(int num_steps, std::optional<unsigned int> seed = std::nullopt);
  walk(const std::string &path, std::optional<unsigned int> seed = std::nullopt);

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk() = default;

  point<Dim, Simd> operator[](int i) const { return steps_[i]; }

  int num_steps() const { return steps_.size(); }

  point<Dim, Simd> endpoint() const override { return steps_.back(); }

  std::optional<std::vector<point<Dim, Simd>>> try_pivot(int step, const transform<Dim, Simd> &trans) const;

  std::pair<int, std::optional<std::vector<point<Dim, Simd>>>> try_rand_pivot() const;

  bool rand_pivot(bool fast = false) override;

  bool rand_pivot(int num_workers);

  bool self_avoiding() const override;

  void export_csv(const std::string &path) const override;

protected:
  std::vector<point<Dim, Simd>> steps_;
  boost::unordered_flat_map<point<Dim, Simd>, int, point_hash> occupied_;

  mutable std::mt19937 rng_;
  mutable std::uniform_int_distribution<int> dist_;

  void do_pivot(int step, std::vector<point<Dim, Simd>> &new_points);

  point<Dim, Simd> pivot_point(int step, int i, const transform<Dim, Simd> &trans) const;
};

} // namespace pivot
