#pragma once

#include <cstdlib>
#include <span>
#include <vector>

#include <string>

namespace pivot {

struct box;
class transform;

class point {

public:
  point(int dim);

  point(std::vector<int> &&coords);

  point(const point &p);

  point &operator=(const point &p);

  static point unit(int dim, int i);

  int dim() const;

  int operator[](int i) const;

  bool operator==(const point &p) const;

  bool operator!=(const point &p) const;

  point operator+(const point &p) const;

  point &operator+=(const point &p);

  point &operator*=(const transform &t);

  box operator+(const box &b) const;

  point operator-(const point &p) const;

  friend point operator*(int k, const point &p);

  std::string to_string() const;

private:
  std::vector<int> coords_;
};

struct interval {
  int left_;
  int right_;

  interval();
  interval(int left, int right);

  bool empty() const;

  std::string to_string() const;
};

struct box {
  std::vector<interval> intervals_;

  box(int dim);

  box(std::vector<interval> &&intervals);

  box(std::span<const point> points);

  box(const box &b);

  box &operator=(const box &b);

  int dim() const;

  bool empty() const;

  // union
  box operator+(const box &b) const;

  box &operator+=(const point &p);

  box &operator+=(const box &b);

  // intersection
  box operator*(const box &b) const;

  box &operator*=(const box &b);

  box &operator*=(const transform &t);

  std::string to_string() const;
};

struct point_hash {
  int num_steps_;

  point_hash(int num_steps);

  std::size_t operator()(const point &p) const;
};

class transform {

public:
  transform(int dim);

  transform(std::vector<int> &&perm, std::vector<int> &&signs);

  transform(const point &p, const point &q);

  static transform rand(int dim);

  int dim() const;

  bool operator==(const transform &t) const;

  point operator*(const point &p) const;

  transform operator*(const transform &t) const;

  box operator*(const box &b) const;

  transform inverse() const;

  std::vector<std::vector<int>> to_matrix() const;

  std::string to_string() const;

private:
  std::vector<int> perm_;
  std::vector<int> signs_;

  friend struct box;
  friend class point;
};

void to_csv(const std::string &path, const std::vector<point> &points);

} // namespace pivot
