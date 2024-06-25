#include <fstream>
#include <stdexcept>
#include <vector>

#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "utils.h"

namespace pivot {

std::vector<point> from_csv(int dim, const std::string &path) {
  std::ifstream file(path);
  std::vector<point> points;
  std::string line;
  while (std::getline(file, line)) {
    std::vector<int> coords;
    coords.reserve(dim);
    size_t start = 0;
    for (int i = 0; i < dim; ++i) {
      size_t end = line.find(',', start);
      if (end == std::string::npos && i < dim - 1) {
        throw std::invalid_argument("Invalid CSV format at line " + std::to_string(points.size()));
      }
      coords.push_back(std::stoi(line.substr(start, end - start)));
      start = end + 1;
    }
    points.push_back(point(coords));
  }
  return points;
}

void to_csv(const std::string &path, const std::vector<point> &points) {
  // TODO: check path exists
  int dim = points[0].dim();
  std::ofstream file(path);
  for (const auto &p : points) {
    for (int i = 0; i < dim - 1; ++i) {
      file << p[i] << ",";
    }
    file << p[dim - 1] << std::endl;
  }
}

std::vector<point> line(int dim, int num_steps) {
  std::vector<point> steps;
  steps.reserve(num_steps);
  for (int i = 0; i < num_steps; ++i) {
    steps.push_back((i + 1) * point::unit(dim, 0));
  }
  return steps;
}

} // namespace pivot
