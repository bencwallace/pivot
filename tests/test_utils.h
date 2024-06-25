#pragma once

#include <random>
#include <vector>

#include "lattice.h"

using namespace pivot;

std::vector<pivot::point> random_walk(int dim, int num_sites) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, dim - 1);

    std::vector<pivot::point> steps;
    steps.push_back(pivot::point::unit(dim, 0));
    for (int i = 1; i < num_sites; ++i) {
        steps.push_back(steps.back() + pivot::point::unit(dim, dist(gen)));
    }
    return steps;
}

std::vector<pivot::point> set_start(const pivot::point &start, const std::vector<pivot::point> &steps) {
    std::vector<pivot::point> shifted;
    for (const auto &step : steps) {
        shifted.push_back(step - steps[0] + start);
    }
    return shifted;
}
