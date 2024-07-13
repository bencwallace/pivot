#pragma once

#include "lattice.h"

namespace pivot {

template <int Dim> class walk_base {

public:
  virtual ~walk_base() = default;

  virtual bool rand_pivot(int num_workers, bool fast) = 0;
  bool rand_pivot(bool fast) { return rand_pivot(0, fast); }

  virtual bool self_avoiding() const = 0;

  virtual void export_csv(const std::string &path) const = 0;

  virtual point<Dim> endpoint() const = 0;
};

} // namespace pivot
