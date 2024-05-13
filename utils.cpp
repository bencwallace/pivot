#include <bitset>

#include "utils.h"

point::point() : x_(0), y_(0) {}

point::point(int x, int y) : x_(x), y_(y) {}

int point::x() const {
    return x_;
}

int point::y() const {
    return y_;
}

bool point::operator==(const point &p) const {
    return x_ == p.x() && y_ == p.y();
}

bool point::operator!=(const point &p) const {
    return x_ != p.x() || y_ != p.y();
}

point point::operator+(const point &p) const {
    return point(x_ + p.x(), y_ + p.y());
}

point point::operator-(const point &p) const {
    return point(x_ - p.x(), y_ - p.y());
}

std::size_t point_hash::operator()(const point &p) const {
    return std::hash<int>()(p.x()) ^ std::hash<int>()(p.y());
}

rot::rot() : cos_(1), sin_(0) {}

rot::rot(angle a) {
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

rot rot::rand() {
    auto r = 1 + std::rand() % 3;
    return rot(static_cast<angle>(r));
}

point rot::operator*(const point &p) const {
    return point(cos_ * p.x() - sin_ * p.y(), sin_ * p.x() + cos_ * p.y());
}
