#include <iostream>

#include <boost/program_options.hpp>

#include "walk.h"
#include "walk_base.h"
#include "walk_tree.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
  int num_steps;
  int iters;
  bool fast;
  int num_workers;
  bool require_success;
  bool verify;
  std::string out_dir;
  int seed;

  po::options_description desc("Allowed options");
  auto op = desc.add_options();
  op("help", "produce help message");
  op("steps", po::value<int>(&num_steps), "number of steps");
  op("iters", po::value<int>(&iters), "number of iterations");
  op("fast", po::value<bool>(&fast)->default_value(true), "use tree-walk implementation");
  op("workers", po::value<int>(&num_workers)->default_value(0), "number of workers");
  op("success", po::value<bool>(&require_success)->default_value(false), "require success");
  op("verify", po::value<bool>(&verify)->default_value(false), "verify");
  op("out", po::value<std::string>(&out_dir)->default_value(""), "output directory");
  op("seed", po::value<int>(&seed)->default_value(time(nullptr)), "seed");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  pivot::walk_base *w;
  if (fast) {
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
