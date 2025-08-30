#pragma once

#include "lattice.h"

namespace pivot {

template <class P> class walk_base {

public:
  virtual ~walk_base() = default;

  virtual bool rand_pivot(bool fast) = 0;

  virtual bool self_avoiding() const = 0;

  virtual void export_csv(const std::string &path) const = 0;

  virtual P endpoint() const = 0;
};

} // namespace pivot
