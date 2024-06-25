#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include "box.hpp"
#include "lattice.h"
#include "transform.hpp"

namespace pivot {

#define BOX_INST(z, n, data) template struct box<n>;
#define TRANSFORM_INST(z, n, data) template class transform<n>;

// cppcheck-suppress syntaxError
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, BOX_INST, ~)
BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, TRANSFORM_INST, ~)

} // namespace pivot
