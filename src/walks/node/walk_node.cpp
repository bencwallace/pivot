#include <stdexcept>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "walk_node.h"

#include "ctors.hpp"
#include "graphviz.hpp"
#include "pivot.hpp"

namespace pivot {

template <int Dim> bool walk_node<Dim>::operator==(const walk_node &other) const {
  if (is_leaf() && other.is_leaf()) {
    return true;
  }
  return id_ == other.id_ && num_sites_ == other.num_sites_ && symm_ == other.symm_ && bbox_ == other.bbox_ &&
         end_ == other.end_ && *left_ == *other.left_ && *right_ == *other.right_;
}

template <int Dim> bool walk_node<Dim>::is_leaf() const { return left_ == nullptr && right_ == nullptr; }

template <int Dim> std::vector<point<Dim>> walk_node<Dim>::steps() const {
  std::vector<point<Dim>> result;
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

#define INTERSECT_INST(z, n, data)                                                                                     \
  template bool intersect<n>(const walk_node<n> *l_walk, const walk_node<n> *r_walk, const point<n> &l_anchor,         \
                             const point<n> &r_anchor, const transform<n> &l_symm, const transform<n> &r_symm);
#define WALK_NODE_INST(z, n, data) template class walk_node<n>;

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, INTERSECT_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, WALK_NODE_INST, ~)

} // namespace pivot
