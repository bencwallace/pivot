#pragma once

#include <future>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <unordered_map>

#include <boost/unordered/unordered_map.hpp>

#include "utils.h"
#include "walk_base.h"

namespace pivot {

template <int Dim> class walk : public walk_base<Dim> {

public:
  walk(int num_steps, std::optional<unsigned int> seed = std::nullopt)
      : steps_(num_steps), occupied_(num_steps, point_hash(num_steps)) {
    for (int i = 0; i < num_steps; ++i) {
      steps_[i] = i * point<Dim>::unit(0);
      occupied_[steps_[i]] = i;
    }

    rng_ = std::mt19937(seed.value_or(std::random_device()()));
    dist_ = std::uniform_int_distribution<int>(0, num_steps - 1);
  }

  walk(const walk &w) = delete;
  walk(walk &&w) = delete;
  walk &operator=(const walk &w) = delete;

  ~walk() = default;

  int num_steps() const { return steps_.size(); }

  point<Dim> endpoint() const override { return steps_.back(); }

  std::optional<std::vector<point<Dim>>> try_pivot(int step, const transform<Dim> &trans) const {
    std::vector<point<Dim>> new_points(num_steps() - step - 1);
    for (int i = step + 1; i < num_steps(); ++i) {
      auto q = pivot_point(step, i, trans);
      auto it = occupied_.find(q);
      if (it != occupied_.end() && it->second <= step) {
        return {};
      }
      new_points[i - step - 1] = q;
    }
    return new_points;
  }

  std::pair<int, std::optional<std::vector<point<Dim>>>> try_rand_pivot() const {
    auto step = dist_(rng_);
    auto r = transform<Dim>::rand(rng_);
    return {step, try_pivot(step, r)};
  }

  bool rand_pivot() override {
    auto [step, new_points] = try_rand_pivot();
    if (!new_points) {
      return false;
    }
    do_pivot(step, new_points.value());
    return true;
  }

  bool rand_pivot(int num_workers) {
    if (num_workers == 0) {
      return rand_pivot();
    }

    std::vector<int> steps(num_workers);
    std::vector<std::optional<std::vector<point<Dim>>>> proposals(num_workers);
    std::vector<std::future<std::pair<int, std::optional<std::vector<point<Dim>>>>>> futures(num_workers);
    for (int i = 0; i < num_workers; ++i) {
      futures[i] = std::async(&walk::try_rand_pivot, this);
    }
    for (int i = 0; i < num_workers; ++i) {
      auto [step, new_points] = futures[i].get();
      steps[i] = step;
      proposals[i] = new_points;
    }
    bool success = false;
    for (int i = 0; i < num_workers; ++i) {
      auto step = steps[i];
      auto new_points = proposals[i];
      if (new_points) {
        do_pivot(step, new_points.value());
        success = true;
        break;
      }
    }
    return success;
  }

  bool self_avoiding() const override {
    for (int i = 0; i < num_steps(); ++i) {
      for (int j = i + 1; j < num_steps(); ++j) {
        if (steps_[i] == steps_[j]) {
          return false;
        }
      }
    }
    return true;
  }

  void export_csv(const std::string &path) const override { return to_csv(path, steps_); }

protected:
  std::vector<point<Dim>> steps_;
  boost::unordered_map<point<Dim>, int, point_hash> occupied_;

  mutable std::mt19937 rng_;
  mutable std::uniform_int_distribution<int> dist_;

  void do_pivot(int step, std::vector<point<Dim>> &new_points) {
    for (auto it = steps_.begin() + step + 1; it != steps_.end(); ++it) {
      occupied_.erase(*it);
    }
    for (int i = step + 1; i < num_steps(); ++i) {
      steps_[i] = new_points[i - step - 1];
      occupied_[steps_[i]] = i;
    }
  }

  point<Dim> pivot_point(int step, int i, const transform<Dim> &trans) const {
    auto p = steps_[step];
    return p + trans * (steps_[i] - p);
  }
};

} // namespace pivot
