#include <vector>

#include "lattice.h"

namespace pivot {

template <int Dim> std::vector<point<Dim>> from_csv(const std::string &path);

template <int Dim> void to_csv(const std::string &path, const std::vector<point<Dim>> &points);

template <int Dim> std::vector<point<Dim>> line(int num_steps);

} // namespace pivot
