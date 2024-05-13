#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

const int Dim = 2;

class point {

public:
    point() : x_(0), y_(0) {}

    point(int x, int y) : x_(x), y_(y) {}

    int x() const {
        return x_;
    }

    int y() const {
        return y_;
    }

    bool operator==(const point &p) const {
        return x_ == p.x() && y_ == p.y();
    }

    bool operator!=(const point &p) const {
        return x_ != p.x() || y_ != p.y();
    }

    point operator+(const point &p) {
        return point(x_ + p.x(), y_ + p.y());
    }

    point operator-(const point &p) {
        return point(x_ - p.x(), y_ - p.y());
    }

private:
    int x_;
    int y_;

};

struct point_hash {
    std::size_t operator()(const point &p) const {
        return std::hash<int>()(p.x()) ^ std::hash<int>()(p.y());
    }
};

enum angle { zero, ninety, one_eighty, two_seventy };

class rot {

public:
    rot() : cos_(1), sin_(0) {}

    rot(angle a) {
        switch (a) {
            case zero:
                cos_ = 1;
                sin_ = 0;
                break;
            case ninety:
                cos_ = 0;
                sin_ = 1;
                break;
            case one_eighty:
                cos_ = -1;
                sin_ = 0;
                break;
            case two_seventy:
                cos_ = 0;
                sin_ = -1;
                break;
        }
    }

    static rot rand() {
        auto r = 1 + std::rand() % 3;
        return rot(static_cast<angle>(r));
    }

    point operator*(const point &p) {
        return point(cos_ * p.x() - sin_ * p.y(), sin_ * p.x() + cos_ * p.y());
    }

private:
    int cos_;
    int sin_;

};

class walk {

public:
    walk(int num_steps) : num_steps_(num_steps) {
        steps_ = new point[num_steps];
        for (int i = 0; i < num_steps; ++i) {
            steps_[i] = point(i, 0);
        }
    }

    ~walk() {
        delete[] steps_;
    }

    point *try_pivot(int step, rot r) {
        std::unordered_set<point, point_hash> init_segment;
        for (int i = 0; i <= step; ++i) {
            init_segment.insert(steps_[i]);
        }

        point *new_points = new point[num_steps_ - step - 1];
        for (int i = step + 1; i < num_steps_; ++i) {
            auto q = pivot_point(step, i, r);
            if (init_segment.find(q) != init_segment.end()) {
                delete[] new_points;
                return nullptr;
            }
            new_points[i - step - 1] = q;
        }
        return new_points;
    }

    bool pivot(int step, rot r) {
        auto new_points = try_pivot(step, r);
        if (new_points == nullptr) {
            return false;
        }
        for (int i = step + 1; i < num_steps_; ++i) {
            steps_[i] = new_points[i - step - 1];
        }
        delete[] new_points;
        return true;
    }

    bool rand_pivot() {
        auto step = std::rand() % num_steps_;
        auto r = rot::rand();
        return pivot(step, r);
    }

    bool self_avoiding() {
        for (int i = 0; i < num_steps_; ++i) {
            for (int j = i + 1; j < num_steps_; ++j) {
                if (steps_[i] == steps_[j]) {
                    return false;
                }
            }
        }
        return true;
    }

    void export_csv(std::string path) {
        std::ofstream file(path);
        for (int i = 0; i < num_steps_; ++i) {
            file << steps_[i].x() << "," << steps_[i].y() << std::endl;
        }
    }

private:
    int num_steps_;
    point *steps_;

    point pivot_point(int step, int i, rot r) {
        auto p = steps_[step];
        return p + r * (steps_[i] - p);
    }

};

int main(int argc, char **argv) {
    int num_steps;
    int iters;
    bool require_success;
    int seed;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("steps", po::value<int>(&num_steps), "number of steps")
        ("iters", po::value<int>(&iters), "number of iterations")
        ("success", po::value<bool>(&require_success), "require success")
        ("seed", po::value<int>(&seed)->default_value(time(nullptr)), "seed");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    walk w(num_steps);

    int num_success = 0;
    int num_iter = 0;
    auto interval = static_cast<int>(std::pow(10, std::floor(std::log10(iters / 10))));
    std::srand(seed);
    while (true) {
        if (num_iter % interval == 0) {
            std::cout << "Iterations: " << num_iter << " / Successes: " << num_success << " / Success rate: " << num_success / static_cast<float>(num_iter) << std::endl;
        }

        num_success += w.rand_pivot();
        ++num_iter;

        if (require_success) {
            if (num_success == iters) {
                break;
            }
        } else if (num_iter == iters) {
            break;
        }
    }

    assert(w.self_avoiding());
    w.export_csv("walk.csv");
}
