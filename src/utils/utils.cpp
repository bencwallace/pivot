#include <fstream>
#include <stdexcept>
#include <vector>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "utils.h"

namespace pivot {

template <int Dim> std::vector<point<Dim>> from_csv(const std::string &path) {
  std::ifstream file(path);
  std::vector<point<Dim>> points;
  std::string line;
  while (std::getline(file, line)) {
    std::array<int, Dim> coords;
    size_t start = 0;
    for (int i = 0; i < Dim; ++i) {
      size_t end = line.find(',', start);
      if (end == std::string::npos && i < Dim - 1) {
        throw std::invalid_argument("Invalid CSV format at line " + std::to_string(points.size()));
      }
      coords[i] = std::stoi(line.substr(start, end - start));
      start = end + 1;
    }
    points.push_back(point<Dim>(coords));
  }
  return points;
}

template <int Dim> void to_csv(const std::string &path, const std::vector<point<Dim>> &points) {
  // TODO: check path exists
  std::ofstream file(path);
  for (const auto &p : points) {
    for (int i = 0; i < Dim - 1; ++i) {
      file << p[i] << ",";
    }
    file << p[Dim - 1] << std::endl;
  }
}

template <int Dim> std::vector<point<Dim>> line(int num_steps) {
  std::vector<point<Dim>> steps(num_steps);
  for (int i = 0; i < num_steps; ++i) {
    steps[i] = (i + 1) * point<Dim>::unit(0);
  }
  return steps;
}

#define FROM_CSV_INST(z, n, data) template std::vector<point<n>> from_csv<n>(const std::string &path);
#define TO_CSV_INST(z, n, data) template void to_csv<n>(const std::string &path, const std::vector<point<n>> &points);
#define LINE_INST(z, n, data) template std::vector<point<n>> line(int num_steps);

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TO_CSV_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, FROM_CSV_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, LINE_INST, ~)

} // namespace pivot
