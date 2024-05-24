#include <fstream>
#include <future>
#include <unordered_set>
#include <vector>

#include "utils.h"
#include "walk.h"

namespace pivot {

walk::walk(int num_steps) : steps_(num_steps), occupied_(num_steps, point_hash(num_steps)) {
  for (int i = 0; i < num_steps; ++i) {
    steps_[i] = point<2>({i, 0});
    occupied_[steps_[i]] = i;
  }
}

int walk::num_steps() const { return steps_.size(); }

point<2> walk::endpoint() const { return steps_.back(); }

std::pair<int, std::optional<std::vector<point<2>>>> walk::try_rand_pivot() const {
  auto step = std::rand() % num_steps();
  auto r = transform<2>::rand();
  return {step, try_pivot(step, r)};
}

bool walk::rand_pivot() {
  auto [step, new_points] = try_rand_pivot();
  if (!new_points) {
    return false; // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
  }
  new_points.value();
  do_pivot(step, new_points.value());
  return true;
}

bool walk::rand_pivot(int num_workers) {
  if (num_workers == 0) {
    return rand_pivot();
  }

  std::vector<int> steps(num_workers);
  std::vector<std::optional<std::vector<point<2>>>> proposals(num_workers);
  std::vector<std::future<std::pair<int, std::optional<std::vector<point<2>>>>>> futures(num_workers);
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

bool walk::self_avoiding() const {
  for (int i = 0; i < num_steps(); ++i) {
    for (int j = i + 1; j < num_steps(); ++j) {
      if (steps_[i] == steps_[j]) {
        return false;
      }
    }
  }
  return true;
}

void walk::export_csv(const std::string &path) const { return to_csv(path, steps_); }

point<2> walk::pivot_point(int step, int i, const transform<2> &trans) const {
  auto p = steps_[step];
  return p + trans * (steps_[i] - p);
}

std::optional<std::vector<point<2>>> walk::try_pivot(int step, const transform<2> &trans) const {
  std::vector<point<2>> new_points(num_steps() - step - 1);
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

void walk::do_pivot(int step, std::vector<point<2>> &new_points) {
  for (auto it = steps_.begin() + step + 1; it != steps_.end(); ++it) {
    occupied_.erase(*it);
  }
  for (int i = step + 1; i < num_steps(); ++i) {
    steps_[i] = new_points[i - step - 1];
    occupied_[steps_[i]] = i;
  }
}

} // namespace pivot
