#include "lattice_simd.h"

namespace pivot {

transform<2, true>::transform() : perm_(_mm_setr_epi32(0, 1, 2, 3)), signs_(_mm_set1_epi32(1)) {}

transform<2, true>::transform(const std::array<int, 2> &perm, const std::array<int, 2> &signs)
    : perm_(_mm_setr_epi32(perm[0], perm[1], 2 + perm[0], 2 + perm[1])),
      signs_(_mm_setr_epi32(signs[0], signs[1], signs[0], signs[1])) {}

transform<2, true>::transform(const point<2, true> &p, const point<2, true> &q) : transform() {
  point<2, true> diff = q - p;
  int idx = -1;
  for (int i = 0; i < 2; ++i) {
    if (std::abs(diff[i]) == 1) {
      if (idx == -1) {
        idx = i;
      } else {
        throw std::invalid_argument("Points are not adjacent");
      }
    }
  }
  if (idx == -1) {
    throw std::invalid_argument("Points are not adjacent");
  }

  perm_ = insert_epi32(perm_, idx, 0);
  perm_ = insert_epi32(perm_, 0, idx);
  signs_ = insert_epi32(signs_, -diff[idx], 0);
  signs_ = insert_epi32(signs_, diff[idx], idx);
}

transform<2, true> transform<2, true>::rand() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return rand(gen);
}

bool transform<2, true>::operator==(const transform &t) const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, t.signs_)) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, t.perm_)) == 0xFFFF;
}

point<2, true> transform<2, true>::operator*(const point<2, true> &p) const {
  return _mm_sign_epi32(permutevar_epi32(p.data(), perm_), signs_);
}

transform<2, true> transform<2, true>::operator*(const transform<2, true> &t) const {
  __m128i perm = permutevar_epi32(perm_, t.perm_);
  __m128i signs = _mm_sign_epi32(permutevar_epi32(t.signs_, perm_), signs_);
  return {perm, signs};
}

box<2, true> transform<2, true>::operator*(const box<2, true> &b) const {
  __m128i pairs = _mm_sign_epi32(permutevar_epi32(b.data(), perm_), signs_);
  return sort_bounds(pairs);
}

bool transform<2, true>::is_identity() const {
  return _mm_movemask_epi8(_mm_cmpeq_epi32(signs_, _mm_set1_epi32(1))) == 0xFFFF &&
         _mm_movemask_epi8(_mm_cmpeq_epi32(perm_, _mm_setr_epi32(0, 1, 2, 3))) == 0xFFFF;
}

transform<2, true> transform<2, true>::inverse() const {
  // In general, the inverse is given by signs S' and permutations P' such
  // that P' = P^-1 and S' = S P. The latter's components can be obtained by
  // viewing S as a vector and applying P to it (i.e. permuting it). Moreover,
  // in 2D, P^-1 is the same as P.
  return transform(perm_, permutevar_epi32(signs_, perm_));
}

std::array<std::array<int, 2>, 2> transform<2, true>::to_matrix() const {
  std::array<std::array<int, 2>, 2> mat = {{{0, 0}, {0, 0}}};
  for (int i = 0; i < 2; ++i) {
    int p = extract_epi32(perm_, i);
    mat[p][i] = extract_epi32(signs_, p);
  }
  return mat;
}

// TODO: double-check
std::string transform<2, true>::to_string() const {
  auto mat = to_matrix();
  std::string s = "[";
  for (int i = 0; i < 2; ++i) {
    s += "[";
    for (int j = 0; j < 2; ++j) {
      s += std::to_string(mat[i][j]);
      if (j < 1) {
        s += ", ";
      }
    }
    s += "]";
    if (i < 1) {
      s += ", ";
    }
  }
  s += "]";
  return s;
}

} // namespace pivot