#include <iostream>
#include <memory>

#include <CLI/CLI.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <random>

#include "defines.h"
#include "walk.h"
#include "walk_base.h"
#include "walk_tree.h"

template <int Dim>
int main_loop(int num_steps, int iters, bool naive, int seed, bool require_success, bool verify,
              const std::string &out_dir) {
  std::unique_ptr<pivot::walk_base<Dim>> w;
  if (!naive) {
    w = std::make_unique<pivot::walk_tree<Dim>>(num_steps, seed);
  } else {
    w = std::make_unique<pivot::walk<Dim>>(num_steps, seed);
  }

  std::vector<pivot::point<Dim>> endpoints;
  if (require_success) {
    endpoints.reserve(iters);
  }

  int num_success = 0;
  int num_iter = 0;
  auto interval = static_cast<int>(std::pow(10, std::floor(std::log10(std::max(iters / 10, 1)))));
  while (true) {
    if (num_iter % interval == 0) {
      std::cout << "Iterations: " << num_iter << " / Successes: " << num_success
                << " / Success rate: " << num_success / static_cast<float>(num_iter) << std::endl;
    }
    if (require_success) {
      if (num_success == iters) {
        break;
      }
    } else if (num_iter == iters) {
      break;
    }

    auto success = w->rand_pivot();
    if (success) {
      endpoints.push_back(w->endpoint());
      ++num_success;
    }
    ++num_iter;
  }
  if (!out_dir.empty()) {
    std::cout << "Saving to: " << out_dir << '\n';
    w->export_csv(out_dir + "/walk.csv");
    pivot::to_csv(out_dir + "/endpoints.csv", endpoints);
  }
  if (verify) {
    std::cout << "Verifying self-avoiding\n";
    if (!w->self_avoiding()) {
      std::cerr << "Walk is not self-avoiding\n";
      return 1;
    }
  }
  return 0;
}

#define CASE_MACRO(z, n, data)                                                                                         \
  case n:                                                                                                              \
    return main_loop<n>(num_steps, iters, naive, seed, require_success, verify, out_dir);                              \
    break;

int main(int argc, char **argv) {
  int dim;
  int num_steps;
  int iters;
  bool naive{false};
  int num_workers{0};
  bool require_success{false};
  bool verify{false};
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
  app.add_option("--out", out_dir, "output directory");
  app.add_option("--seed", seed, "seed")->default_val(std::random_device()());

  CLI11_PARSE(app, argc, argv);

  switch (dim) {
    BOOST_PP_REPEAT_FROM_TO(1, DIMS_UB, CASE_MACRO, ~)
  default:
    std::cerr << "Invalid dimension: " << dim << '\n';
    return 1;
  }
}
