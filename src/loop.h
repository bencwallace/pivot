#include <iostream>
#include <memory>
#include <string>

#include "logging.h"
#include "utils.h"
#include "walk.h"
#include "walk_tree.h"

template <int Dim>
int main_loop(int num_steps, int iters, bool naive, bool fast, int seed, bool require_success, bool verify,
              const std::string &in_path, const std::string &out_dir) {
  pivot::configureLogger();
  auto logger = spdlog::get("pivot");

  std::unique_ptr<pivot::walk_base<Dim>> w;
  if (!naive) {
    if (in_path.empty()) {
      w = std::make_unique<pivot::walk_tree<Dim>>(num_steps, seed);
    } else {
      w = std::make_unique<pivot::walk_tree<Dim>>(in_path, seed);
    }
  } else {
    w = std::make_unique<pivot::walk<Dim>>(num_steps, seed);
  }

  std::vector<pivot::point<Dim>> endpoints;
  if (require_success) {
    endpoints.reserve(iters);
  }

  int num_success = 0;
  int total_success = 0;
  int num_iter = 0;
  auto interval = static_cast<int>(std::pow(10, std::floor(std::log10(std::max(iters / 10, 1)))));
  logger->info("Starting loop with {} iterations", iters);
  while (true) {
    if (num_iter % interval == 0) {
      logger->info("Iterations: {}/{} / Successes: {} / Success rate: {}", num_iter, iters, total_success,
                   num_success / static_cast<float>(interval));
      num_success = 0;
    }
    if (require_success) {
      if (total_success == iters) {
        break;
      }
    } else if (num_iter == iters) {
      break;
    }

    auto success = w->rand_pivot(fast);
    if (success) {
      endpoints.push_back(w->endpoint());
      ++num_success;
      ++total_success;
    }
    ++num_iter;
  }
  logger->info("Iterations: {}/{} / Successes: {} / Success rate: {}", num_iter, iters, total_success,
               num_success / static_cast<float>(interval));
  if (!out_dir.empty()) {
    logger->info("Saving to: {}", out_dir);
    w->export_csv(out_dir + "/walk.csv");
    pivot::to_csv(out_dir + "/endpoints.csv", endpoints);
  }
  if (verify) {
    logger->info("Verifying self-avoiding");
    if (!w->self_avoiding()) {
      logger->error("Walk is not self-avoiding");
      return 1;
    }
  }
  return 0;
}
