#include <fstream>
#include <future>
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

std::pair<int, point *> walk::try_rand_pivot() {
    auto step = std::rand() % num_steps_;
    auto r = rot::rand();
    return {step, try_pivot(step, r)};
}

void walk::set(int i, point p) {
    steps_[i] = p;
}

bool walk::pivot(int step, rot r) {
    auto new_points = try_pivot(step, r);
    if (new_points == nullptr) {
        return false;
    }
    for (int i = step + 1; i < num_steps_; ++i) {
        set(i, new_points[i - step - 1]);
    }
    delete[] new_points;
    return true;
}

bool walk::rand_pivot() {
    auto [step, new_points] = try_rand_pivot();
    if (new_points == nullptr) {
        return false;
    }
    for (int i = step + 1; i < num_steps_; ++i) {
        set(i, new_points[i - step - 1]);
    }
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
            // TODO: this seems to be a bigger bottleneck than expected
            for (int j = step + 1; j < num_steps_; ++j) {
                set(j, new_points[j - step - 1]);
            }
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

/* occupied_walk */

occupied_walk::occupied_walk(int num_steps) : walk(num_steps) {
    for (int i = 0; i < num_steps; ++i) {
        occupied_[steps_[i]] = {i};
    }
}

point *occupied_walk::try_pivot(int step, rot r) {
    point *new_points = new point[num_steps_ - step - 1];
    for (int i = step + 1; i < num_steps_; ++i) {
        auto q = pivot_point(step, i, r);
        auto it = occupied_.find(q);
        if (it != occupied_.end() && *it->second.cbegin() <= step) {
            delete[] new_points;
            return nullptr;
        }
        new_points[i - step - 1] = q;
    }
    return new_points;
}

void occupied_walk::set(int i, point p) {
    auto it = occupied_.find(steps_[i]);
    if (it != occupied_.end()) {
        it->second.erase(i);
        if (it->second.empty()) {
            occupied_.erase(it);
        }
    }

    steps_[i] = p;

    auto jt = occupied_.find(p);
    if (jt != occupied_.end()) {
        jt->second.insert(i);
    } else {
        occupied_[p] = {i};
    }
}
