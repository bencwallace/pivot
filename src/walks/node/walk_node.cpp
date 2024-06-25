#include <stdexcept>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "walk_node.h"

namespace pivot {

bool walk_node::operator==(const walk_node &other) const {
  if (is_leaf() && other.is_leaf()) {
    return true;
  }
  return id_ == other.id_ && num_sites_ == other.num_sites_ && symm_ == other.symm_ && bbox_ == other.bbox_ &&
         end_ == other.end_ && *left_ == *other.left_ && *right_ == *other.right_;
}

bool walk_node::is_leaf() const { return left_ == nullptr && right_ == nullptr; }

std::vector<point> walk_node::steps() const {
  std::vector<point> result;
  if (is_leaf()) {
    result.push_back(end_);
    return result;
  }

  auto left_steps = left_->steps();
  result.insert(result.begin(), left_steps.begin(), left_steps.end());

  auto right_steps = right_->steps();
  for (auto &step : right_steps) {
    result.push_back(left_->end_ + symm_ * step);
  }

  return result;
}

} // namespace pivot
