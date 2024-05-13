#include <fstream>
#include <unordered_set>

#include "walk.h"

walk::walk(int num_steps) : num_steps_(num_steps) {
    steps_ = new point[num_steps];
    for (int i = 0; i < num_steps; ++i) {
        steps_[i] = point(i, 0);
    }
}

walk::~walk() {
    delete[] steps_;
}

point *walk::try_pivot(int step, rot r) {
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

bool walk::pivot(int step, rot r) {
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

bool walk::rand_pivot() {
    auto step = std::rand() % num_steps_;
    auto r = rot::rand();
    return pivot(step, r);
}

bool walk::self_avoiding() {
    for (int i = 0; i < num_steps_; ++i) {
        for (int j = i + 1; j < num_steps_; ++j) {
            if (steps_[i] == steps_[j]) {
                return false;
            }
        }
    }
    return true;
}

void walk::export_csv(std::string path) {
    std::ofstream file(path);
    for (int i = 0; i < num_steps_; ++i) {
        file << steps_[i].x() << "," << steps_[i].y() << std::endl;
    }
}

point walk::pivot_point(int step, int i, rot r) {
    auto p = steps_[step];
    return p + r * (steps_[i] - p);
}

