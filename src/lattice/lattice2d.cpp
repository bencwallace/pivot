#include "lattice2d.h"

static inline int32_t extract_epi32(__m128i data, size_t i) {
  switch (i) {
  case 0:
    return _mm_extract_epi32(data, 0);
  case 1:
    return _mm_extract_epi32(data, 1);
  case 2:
    return _mm_extract_epi32(data, 2);
  case 3:
    return _mm_extract_epi32(data, 3);
  default:
    return 0;
  }
}

static inline __m128i insert_epi32(__m128i data, int32_t val, size_t i) {
  switch (i) {
  case 0:
    return _mm_insert_epi32(data, val, 0);
  case 1:
    return _mm_insert_epi32(data, val, 1);
  case 2:
    return _mm_insert_epi32(data, val, 2);
  case 3:
    return _mm_insert_epi32(data, val, 3);
  default:
    return data;
  }
}

namespace pivot {

point2d::point2d() : data_(_mm_setzero_si128()) {}

point2d::point2d(std::array<int32_t, 2> coords) : data_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

point2d point2d::unit() { return point2d(std::array{1, 0}); }

int32_t point2d::operator[](size_t i) const { return extract_epi32(data_, i); }

bool point2d::operator==(const point2d &other) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(data_, other.data_)) == 0xFFFF;
}

bool point2d::operator!=(const point2d &other) const { return !(*this == other); }

point2d point2d::operator+(const point2d &other) const { return _mm_add_epi32(data_, other.data_); }

point2d point2d::operator-(const point2d &other) const { return _mm_sub_epi32(data_, other.data_); }

point2d &point2d::operator*=(int32_t k) {
  data_ = _mm_mullo_epi32(_mm_set1_epi32(k), data_);
  return *this;
}

std::string point2d::to_string() const {
  std::string s = "(";
  s += std::to_string(extract_epi32(data_, 0)) + ", ";
  s += std::to_string(extract_epi32(data_, 1)) + ")";
  return s;
}

} // namespace pivot
