#include "lattice_simd.h"

namespace pivot {

point<2, true>::point() : coords_(_mm_setzero_si128()) {}

point<2, true>::point(__m128i coords) : coords_(coords) {}

point<2, true>::point(const std::array<int, 2> &coords) : coords_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

point<2, true> point<2, true>::unit(int i) {
  point p;
  p.coords_ = insert_epi32(p.coords_, 1, i);
  return p;
}

int point<2, true>::operator[](int i) const { return extract_epi32(coords_, i); }

bool point<2, true>::operator==(const point &p) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(coords_, p.coords_)) == 0xFFFF;
}

bool point<2, true>::operator!=(const point &p) const { return !(*this == p); }

point<2, true> point<2, true>::operator+(const point<2, true> &p) const { return _mm_add_epi32(coords_, p.coords_); }

point<2, true> point<2, true>::operator-(const point &p) const { return _mm_sub_epi32(coords_, p.coords_); }

point<2, true> &point<2, true>::operator*=(int k) {
  coords_ = _mm_mullo_epi32(coords_, _mm_set1_epi32(k));
  return *this;
}

// int point<2, true>::norm() const;

std::string point<2, true>::to_string() const {
  std::string s = "(";
  s += std::to_string(extract_epi32(coords_, 0));
  s += ", ";
  s += std::to_string(extract_epi32(coords_, 1));
  s += ")";
  return s;
}

} // namespace pivot
