#include <bitset>
#include <stdexcept>

#include "utils.h"

namespace pivot {

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

point_hash::point_hash(int num_steps) : num_steps_(num_steps) {}

std::size_t point_hash::operator()(const point &p) const {
    return p.x() + num_steps_ * p.y();
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

rot::rot(point p, point q) {
    auto dx = q.x() - p.x();
    auto dy = q.y() - p.y();
    if (!(std::abs(dx) == 1 ^ std::abs(dy) == 1)) {
        throw std::invalid_argument("Points are not adjacent");
    }
    if (dx == 0) {
        if (dy == 1) {
            cos_ = 0;
            sin_ = 1;
        } else if (dy == -1) {
            cos_ = 0;
            sin_ = -1;
        }
    } else if (dy == 0) {
        if (dx == 1) {
            cos_ = 1;
            sin_ = 0;
        } else if (dx == -1) {
            cos_ = -1;
            sin_ = 0;
        }
    }
}

rot rot::rand() {
    auto r = 1 + std::rand() % 3;
    return rot(static_cast<angle>(r));
}

point rot::operator*(const point &p) const {
    return point(cos_ * p.x() - sin_ * p.y(), sin_ * p.x() + cos_ * p.y());
}

std::string rot::to_string() const {
    return "(" + std::to_string(cos_) + ", " + std::to_string(sin_) + ")";
}

} // namespace pivot
