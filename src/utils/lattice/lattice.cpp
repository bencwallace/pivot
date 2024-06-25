#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "lattice.h"
#include "transform.hpp"

namespace pivot {

#define TRANSFORM_INST(z, n, data) template class transform<n>;

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TRANSFORM_INST, ~)

} // namespace pivot
