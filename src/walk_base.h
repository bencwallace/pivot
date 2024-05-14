#pragma once

#include <string>

namespace pivot {

class walk_base {

public:
  virtual ~walk_base() = default;

  virtual bool rand_pivot() = 0;

  virtual bool self_avoiding() const = 0;

  virtual void export_csv(const std::string &path) const = 0;

  virtual point endpoint() const = 0;
};

} // namespace pivot
