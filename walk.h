#pragma once

#include <string>

#include "utils.h"

class walk {

public:
    walk(int num_steps);

    ~walk();

    point *try_pivot(int step, rot r);

    bool pivot(int step, rot r);

    bool rand_pivot();

    bool self_avoiding();

    void export_csv(std::string path);

private:
    int num_steps_;
    point *steps_;

    point pivot_point(int step, int i, rot r);

};
