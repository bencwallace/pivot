#pragma once

#include <cstdlib>

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
    std::size_t operator()(const point &p) const;
};

class rot {

public:
    rot();

    rot(angle a);

    static rot rand();

    point operator*(const point &p) const;

private:
    int cos_;
    int sin_;

};
