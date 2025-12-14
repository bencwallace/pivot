#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "box.hpp"
#include "lattice.h"
#include "point.hpp"
#include "transform.hpp"

#ifdef ENABLE_AVX2
#include "box_simd.hpp"
#include "point_simd.hpp"
#include "transform_simd.hpp"
#endif

namespace pivot {

#define BOX_INST(z, n, data) template struct box<n>;
#define POINT_INST(z, n, data) template class point<n>;
#define POINT_HASH_CALL_INST(z, n, data) template std::size_t point_hash::operator()<n>(const point<n> &p) const;
#define TRANSFORM_INST(z, n, data) template class transform<n>;

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, BOX_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, POINT_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, POINT_HASH_CALL_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TRANSFORM_INST, ~)

#ifdef ENABLE_AVX2
template std::size_t point_hash::operator()<2>(const point<2, true> &p) const;
#endif

} // namespace pivot
