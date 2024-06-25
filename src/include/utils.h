#include <vector>

#include "lattice.h"

namespace pivot {

std::vector<point> from_csv(int dim, const std::string &path);

void to_csv(const std::string &path, const std::vector<point> &points);

std::vector<point> line(int dim, int num_steps);

} // namespace pivot
