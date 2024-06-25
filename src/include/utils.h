#include <vector>

#include "lattice.h"

namespace pivot {

template <int Dim> std::vector<point> from_csv(const std::string &path);

template <int Dim> void to_csv(const std::string &path, const std::vector<point> &points);

std::vector<point> line(int dim, int num_steps);

} // namespace pivot
