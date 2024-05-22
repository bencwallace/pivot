#include <iostream>

#include <CLI/CLI.hpp>

#include "walk.h"
#include "walk_base.h"
#include "walk_tree.h"

int main(int argc, char **argv) {
  int num_steps;
  int iters;
  bool naive{false};
  int num_workers{0};
  bool require_success{false};
  bool verify{false};
  std::string out_dir{""};
  int seed;

  CLI::App app{"Implementation of the pivot algorithm"};
  argv = app.ensure_utf8(argv);

  app.add_option("-s,--steps", num_steps, "number of steps")->required();
  app.add_option("-i,--iters", iters, "number of iterations")->required();
  app.add_flag("--naive", naive, "use naive implementation (slower)");
  app.add_option("-w,--workers", num_workers, "number of workers");
  app.add_flag("--success", require_success, "require success");
  app.add_flag("--verify", verify, "verify");
  app.add_option("--out", out_dir, "output directory");
  app.add_option("--seed", seed, "seed")->default_val(time(nullptr));

  CLI11_PARSE(app, argc, argv);

  pivot::walk_base *w;
  if (!naive) {
    w = pivot::walk_tree::line(num_steps);
  } else {
    w = new pivot::walk(num_steps);
  }

  std::vector<pivot::point> endpoints;
  if (require_success) {
    endpoints.reserve(iters);
  }

  int num_success = 0;
  int num_iter = 0;
  auto interval = static_cast<int>(std::pow(10, std::floor(std::log10(std::max(iters / 10, 1)))));
  std::srand(seed);
  while (true) {
    if (num_iter % interval == 0) {
      std::cout << "Iterations: " << num_iter << " / Successes: " << num_success
                << " / Success rate: " << num_success / static_cast<float>(num_iter) << std::endl;
    }

    auto success = w->rand_pivot();
    if (success) {
      endpoints.push_back(w->endpoint());
      ++num_success;
    }
    ++num_iter;

    if (require_success) {
      if (num_success == iters) {
        break;
      }
    } else if (num_iter == iters) {
      break;
    }
  }
  if (!out_dir.empty()) {
    std::cout << "Saving to: " << out_dir << '\n';
    w->export_csv(out_dir + "/walk.csv");
    pivot::to_csv(out_dir + "/endpoints.csv", endpoints);
  }
  if (verify) {
    std::cout << "Verifying self-avoiding\n";
    assert(w->self_avoiding());
  }
}
