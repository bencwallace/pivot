#include <CLI/CLI.hpp>

#include <boost/preprocessor/repeat_from_to.hpp>

#include "loop.h"

#define CASE_MACRO(z, n, data)                                                                                         \
  case n:                                                                                                              \
    return main_loop<n>(num_steps, iters, naive, fast, seed, require_success, verify, in_path, out_dir);               \
    break;

int main(int argc, char **argv) {
  int dim;
  int num_steps;
  int iters;
  bool naive{false};
  std::optional<bool> fast_slow{std::nullopt};
  int num_workers{0};
  bool require_success{false};
  bool verify{false};
  std::string in_path{""};
  std::string out_dir{""};
  unsigned int seed;

  CLI::App app{"Implementation of the pivot algorithm"};
  argv = app.ensure_utf8(argv);

  app.add_option("-d,--dim", dim, "dimension")->required();
  app.add_option("-s,--steps", num_steps, "number of steps")->required();
  app.add_option("-i,--iters", iters, "number of iterations")->required();
  app.add_flag("--naive", naive, "use naive implementation (slower)");
  app.add_flag("--fast,!--slow", fast_slow, "use fast implementation");
  app.add_option("-w,--workers", num_workers, "number of workers");
  app.add_flag("--success", require_success, "require success");
  app.add_flag("--verify", verify, "verify");
  app.add_option("--in", in_path, "input path");
  app.add_option("--out", out_dir, "output directory");
  app.add_option("--seed", seed, "seed")->default_val(std::random_device()());

  CLI11_PARSE(app, argc, argv);
  bool fast;
  if (fast_slow.has_value()) {
    fast = fast_slow.value();
  } else {
    fast = !naive;
  }

  switch (dim) {
    // cppcheck-suppress syntaxError
    BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, CASE_MACRO, ~)
  default:
    std::cerr << "Invalid dimension: " << dim << '\n';
    return 1;
  }
}
