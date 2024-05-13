#pragma once

#include <cstdlib>

#include <string>

namespace pivot {

enum angle { zero, ninety, one_eighty, two_seventy };

class box;

class point {

public:
    point();

    point(int x, int y);

    int x() const;

    int y() const;

    bool operator==(const point &p) const;

    bool operator!=(const point &p) const;

    point operator+(const point &p) const;

    box operator+(const box &b) const;

    point operator-(const point &p) const;

    std::string to_string() const;

private:
    int x_;
    int y_;

};

struct interval {
    int left_;
    int right_;

    interval();
    interval(int left, int right);

    bool empty() const;

    std::string to_string() const;
};

struct box {
    interval x_;
    interval y_;

    box(interval x, interval y);

    box(int n, point *points);

    bool empty() const;

    // union
    box operator+(const box &b) const;

    // intersection
    box operator*(const box &b) const;

    std::string to_string() const;
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

    box operator*(const box &b) const;

    rot inverse() const;

    std::string to_string() const;

private:
    int cos_;
    int sin_;

    rot(int cos, int sin);

};

}
