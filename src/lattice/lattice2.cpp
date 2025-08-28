#include <immintrin.h>

#include "lattice.h"

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

static inline __m128i insert_epi32(__m128i data, int val, size_t i) {
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

template <> class point<2> : boost::multipliable<point<2>, int> {

public:
  point() : coords_(_mm_setzero_si128()) {}

  point(const std::array<int, 2> &coords) : coords_(_mm_setr_epi32(coords[0], coords[1], 0, 0)) {}

  static point unit(int i) {
    __m128i coords = _mm_setzero_si128();
    coords = insert_epi32(coords, 1, i);
    return point(coords);
  }

  int operator[](int i) const { return extract_epi32(coords_, i); }

  bool operator==(const point &p) const { return _mm_movemask_epi8(_mm_cmpeq_epi32(coords_, p.coords_)) == 0xFFFF; }

  bool operator!=(const point &p) const { return !(*this == p); }

  point operator+(const point &p) const { return _mm_add_epi32(coords_, p.coords_); }

  point operator-(const point &p) const { return _mm_sub_epi32(coords_, p.coords_); }

  point &operator*=(int k) {
    coords_ = _mm_mullo_epi32(coords_, _mm_set1_epi32(k));
    return *this;
  }

  __m128i coords() const { return coords_; }

  // TODO: double-check
  int norm() const {
    __m128i squared = _mm_mullo_epi32(coords_, coords_);
    return extract_epi32(_mm_hadd_epi32(squared, squared), 0);
  }

  std::string to_string() const {
    std::string s = "(";
    s += std::to_string(extract_epi32(coords_, 0)) + ", ";
    s += std::to_string(extract_epi32(coords_, 1));
    s += ")";
    return s;
  }

private:
  __m128i coords_;

  point(__m128i coords) : coords_(coords) {}
};

} // namespace pivot
