#include <stdexcept>
#include <vector>

#include "utils.h"

namespace pivot {

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

// TODO: don't distinguish empty intervals
bool interval::operator==(const interval &other) const { return left_ == other.left_ && right_ == other.right_; }

bool interval::operator!=(const interval &other) const { return !(*this == other); }

bool interval::empty() const { return left_ > right_; }

/** @brief Returns the string of the form "[{left_}, {right_}]". */
std::string interval::to_string() const {
  // for some reason, gcc-12 gives -Wrestrict warning if I use the string concatenation operator
  std::string result = "[";
  result.append(std::to_string(left_)).append(", ").append(std::to_string(right_)).append("]");
  return result;
}

} // namespace pivot
