#include <iostream>

#include <boost/program_options.hpp>

#include "walk_tree.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
    int num_steps;
    int iters;
    int num_workers;
    bool require_success;
    bool verify;
    bool save;
    int seed;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("steps", po::value<int>(&num_steps), "number of steps")
        ("iters", po::value<int>(&iters), "number of iterations")
        ("workers", po::value<int>(&num_workers)->default_value(0), "number of workers")
        ("success", po::value<bool>(&require_success)->default_value(false), "require success")
        ("verify", po::value<bool>(&verify)->default_value(false), "verify")
        ("save", po::value<bool>(&save)->default_value(true), "save")
        ("seed", po::value<int>(&seed)->default_value(time(nullptr)), "seed");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    pivot::walk_tree *w = pivot::walk_tree::line(num_steps);

    int num_success = 0;
    int num_iter = 0;
    auto interval = static_cast<int>(std::pow(10, std::floor(std::log10(std::max(iters / 10, 1)))));
    std::srand(seed);
    while (true) {
        if (num_iter % interval == 0) {
            std::cout << "Iterations: " << num_iter << " / Successes: " << num_success << " / Success rate: " << num_success / static_cast<float>(num_iter) << std::endl;
        }

        num_success += w->rand_pivot();
        ++num_iter;

        if (require_success) {
            if (num_success == iters) {
                break;
            }
        } else if (num_iter == iters) {
            break;
        }
    }
    if (save) {
        std::cout << "Saving to walk.csv\n";
        w->export_csv("walk.csv");
    }
    if (verify) {
        std::cout << "Verifying self-avoiding\n";
        assert(w->self_avoiding());
    }
}
