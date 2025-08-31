#pragma once

#include "lattice.h"

namespace pivot {

template <int Dim, bool Simd = false> class walk_base {

public:
  virtual ~walk_base() = default;

  virtual bool rand_pivot(bool fast) = 0;

  virtual bool self_avoiding() const = 0;

  virtual void export_csv(const std::string &path) const = 0;

  virtual point<Dim, Simd> endpoint() const = 0;
};

} // namespace pivot
