#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "defines.h"
#include "utils.h"
#include "walk.h"

namespace pivot {

template <int Dim>
walk<Dim>::walk(const std::vector<point<Dim>> &steps, std::optional<unsigned int> seed)
    : steps_(steps), occupied_(steps.size(), point_hash(steps.size())) {
  for (int i = 0; i < num_steps(); ++i) {
    occupied_[steps_[i]] = i;
  }

  rng_ = std::mt19937(seed.value_or(std::random_device()()));
  dist_ = std::uniform_int_distribution<int>(0, num_steps() - 1);
}

template <int Dim>
walk<Dim>::walk(int num_steps, std::optional<unsigned int> seed) : walk(line<Dim>(num_steps), seed) {}

template <int Dim>
walk<Dim>::walk(const std::string &path, std::optional<unsigned int> seed) : walk(from_csv<Dim>(path), seed) {}

template <int Dim>
std::optional<std::vector<point<Dim>>> walk<Dim>::try_pivot(int step, const transform<Dim> &trans) const {
  if (trans.is_identity()) {
    return {};
  }

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

template <int Dim> std::pair<int, std::optional<std::vector<point<Dim>>>> walk<Dim>::try_rand_pivot() const {
  auto step = dist_(rng_);
  auto r = transform<Dim>::rand(rng_);
  return {step, try_pivot(step, r)};
}

template <int Dim> bool walk<Dim>::rand_pivot(bool fast) {
  if (fast) { // TODO: implement Kennedy algorithm
    throw std::invalid_argument("fast pivot not implemented for naive walk");
  }

  auto [step, new_points] = try_rand_pivot();
  if (!new_points) {
    return false;
  }
  do_pivot(step, new_points.value());
  return true;
}

template <int Dim> bool walk<Dim>::rand_pivot(int num_workers) {
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

template <int Dim> bool walk<Dim>::self_avoiding() const {
  for (int i = 0; i < num_steps(); ++i) {
    for (int j = i + 1; j < num_steps(); ++j) {
      if (steps_[i] == steps_[j]) {
        return false;
      }
    }
  }
  return true;
}

template <int Dim> void walk<Dim>::export_csv(const std::string &path) const { return to_csv(path, steps_); }

template <int Dim> void walk<Dim>::do_pivot(int step, std::vector<point<Dim>> &new_points) {
  for (auto it = steps_.begin() + step + 1; it != steps_.end(); ++it) {
    occupied_.erase(*it);
  }
  for (int i = step + 1; i < num_steps(); ++i) {
    steps_[i] = new_points[i - step - 1];
    occupied_[steps_[i]] = i;
  }
}

template <int Dim> point<Dim> walk<Dim>::pivot_point(int step, int i, const transform<Dim> &trans) const {
  auto p = steps_[step];
  return p + trans * (steps_[i] - p);
}

#define WALK_INST(z, n, data) template class walk<n>;

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, WALK_INST, ~)

} // namespace pivot
