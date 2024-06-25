#include <CLI/CLI.hpp>

#include <boost/preprocessor/repeat_from_to.hpp>

#include "loop.h"

int main(int argc, char **argv) {
  int dim;
  int num_steps;
  int iters;
  bool naive{false};
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
  app.add_option("-w,--workers", num_workers, "number of workers");
  app.add_flag("--success", require_success, "require success");
  app.add_flag("--verify", verify, "verify");
  app.add_option("--in", in_path, "input path");
  app.add_option("--out", out_dir, "output directory");
  app.add_option("--seed", seed, "seed")->default_val(std::random_device()());

  CLI11_PARSE(app, argc, argv);
  return main_loop(dim, num_steps, iters, naive, seed, require_success, verify, in_path, out_dir);
}
