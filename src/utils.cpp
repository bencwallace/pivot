#include <stdexcept>
#include <vector>

#include "utils.h"

namespace pivot {

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

interval::interval() : interval(0, 0) {}

interval::interval(int left, int right) : left_(left), right_(right) {}

bool interval::empty() const { return left_ > right_; }

std::string interval::to_string() const { return "[" + std::to_string(left_) + ", " + std::to_string(right_) + "]"; }

} // namespace pivot
