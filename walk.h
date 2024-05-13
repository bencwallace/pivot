#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include "utils.h"

class walk {

public:
    walk(int num_steps);

    ~walk();

    virtual point *try_pivot(int step, rot r);

    std::pair<int, point *> try_rand_pivot();

    bool pivot(int step, rot r);

    bool rand_pivot();

    bool rand_pivot(int num_workers);

    bool self_avoiding();

    void export_csv(std::string path);

protected:
    int num_steps_;
    point *steps_;

    virtual void set(int i, point p);

    point pivot_point(int step, int i, rot r);

};

class occupied_walk : public walk {

public:
    occupied_walk(int num_steps);

    point *try_pivot(int step, rot r) override;

private:
    std::unordered_map<point, std::set<int>, point_hash> occupied_;

    void set(int i, point p) override;

};
