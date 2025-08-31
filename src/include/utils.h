#include <vector>

#include "lattice.h"

namespace pivot {

template <int Dim, bool Simd> std::vector<point<Dim, Simd>> from_csv(const std::string &path);

template <int Dim, bool Simd> void to_csv(const std::string &path, const std::vector<point<Dim, Simd>> &points);

template <int Dim, bool Simd> std::vector<point<Dim, Simd>> line(int num_steps);

} // namespace pivot
