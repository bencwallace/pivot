#include <fstream>
#include <future>
#include <unordered_set>

#include "walk.h"

walk::walk(int num_steps) : num_steps_(num_steps), occupied_(num_steps, point_hash(num_steps)) {
    steps_ = new point[num_steps];
    for (int i = 0; i < num_steps; ++i) {
        steps_[i] = point(i, 0);
        occupied_[steps_[i]] = i;
    }
}

walk::~walk() {
    delete[] steps_;
}

std::pair<int, point *> walk::try_rand_pivot() {
    auto step = std::rand() % num_steps_;
    auto r = rot::rand();
    return {step, try_pivot(step, r)};
}

bool walk::rand_pivot() {
    auto [step, new_points] = try_rand_pivot();
    if (new_points == nullptr) {
        return false;
    }
    do_pivot(step, new_points);
    delete[] new_points;
    return true;
}

bool walk::rand_pivot(int num_workers) {
    if (num_workers == 0) {
        return rand_pivot();
    }

    int steps[num_workers];
    point *proposals[num_workers];
    std::future<std::pair<int, point *>> futures[num_workers];
    for (int i = 0; i < num_workers; ++i) {
        futures[i] = std::async(&walk::try_rand_pivot, this);
    }
    for (int i = 0; i < num_workers; ++i) {
        auto [step, new_points] = futures[i].get();
        steps[i] = step;
        proposals[i] = new_points;
    }
    bool success = false;
    for (int i = 0; i < num_workers; ++i) {
        auto step = steps[i];
        auto new_points = proposals[i];
        if (!success && new_points != nullptr) {
            do_pivot(step, new_points);
            success = true;
        }
    }
    for (int i = 0; i < num_workers; ++i) {
        if (proposals[i] != nullptr) {
            delete[] proposals[i];
        }
    }
    return success;
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

point *walk::try_pivot(int step, rot r) {
    point *new_points = new point[num_steps_ - step - 1];
    for (int i = step + 1; i < num_steps_; ++i) {
        auto q = pivot_point(step, i, r);
        auto it = occupied_.find(q);
        if (it != occupied_.end() && it->second <= step) {
            delete[] new_points;
            return nullptr;
        }
        new_points[i - step - 1] = q;
    }
    return new_points;
}

void walk::do_pivot(int step, point *new_points) {
    for (int i = step + 1; i < num_steps_; ++i) {
        occupied_.erase(steps_[i]);
    }
    for (int i = step + 1; i < num_steps_; ++i) {
        steps_[i] = new_points[i - step - 1];
        occupied_[steps_[i]] = i;
    }
}
