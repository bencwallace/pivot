#pragma once

#include <cstdlib>

#include <string>

namespace pivot {

enum angle { zero, ninety, one_eighty, two_seventy };

class point {

public:
    point();

    point(int x, int y);

    int x() const;

    int y() const;

    bool operator==(const point &p) const;

    bool operator!=(const point &p) const;

    point operator+(const point &p) const;

    point operator-(const point &p) const;

private:
    int x_;
    int y_;

};

struct point_hash {
    int num_steps_;

    point_hash(int num_steps);

    std::size_t operator()(const point &p) const;
};

class rot {

public:
    rot();

    rot(angle a);

    rot(point p, point q);

    static rot rand();

    point operator*(const point &p) const;

    rot operator*(const rot &r) const;

    std::string to_string() const;

private:
    int cos_;
    int sin_;

    rot(int cos, int sin);

};

}
