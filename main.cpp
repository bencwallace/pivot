#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

const int Dim = 2;

class point {

public:
    point() : x_(0), y_(0) {}

    point(int x, int y) : x_(x), y_(y) {}

    int x() const {
        return x_;
    }

    int y() const {
        return y_;
    }

    bool operator==(const point &p) const {
        return x_ == p.x() && y_ == p.y();
    }

    bool operator!=(const point &p) const {
        return x_ != p.x() || y_ != p.y();
    }

    point operator+(const point &p) {
        return point(x_ + p.x(), y_ + p.y());
    }

    point operator-(const point &p) {
        return point(x_ - p.x(), y_ - p.y());
    }

private:
    int x_;
    int y_;

};

struct point_hash {
    std::size_t operator()(const point &p) const {
        return std::hash<int>()(p.x()) ^ std::hash<int>()(p.y());
    }
};

enum angle { zero, ninety, one_eighty, two_seventy };

class rot {

public:
    rot() : cos_(1), sin_(0) {}

    rot(angle a) {
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

    static rot rand() {
        auto r = std::rand() % 3;
        return rot(static_cast<angle>(r));
    }

    point operator*(const point &p) {
        return point(cos_ * p.x() - sin_ * p.y(), sin_ * p.x() + cos_ * p.y());
    }

private:
    int cos_;
    int sin_;

};

class walk {

public:
    walk(int num_steps) : num_steps_(num_steps) {
        steps_ = new point[num_steps];
        for (int i = 0; i < num_steps; ++i) {
            steps_[i] = point(i, 0);
            occupied_[steps_[i]] = i;
        }
    }

    ~walk() {
        delete[] steps_;
    }

    point *try_pivot(int step, rot r) {
        point *new_points = new point[num_steps_ - step - 1];
        for (int i = step + 1; i < num_steps_; ++i) {
            auto q = pivot_point(step, i, r);
            if (occupied_.find(q) != occupied_.end()) {
                delete[] new_points;
                return nullptr;
            }
            new_points[i - step - 1] = q;
        }
        return new_points;
    }

    bool pivot(int step, rot r) {
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

    bool rand_pivot() {
        auto step = std::rand() % num_steps_;
        auto r = rot::rand();
        return pivot(step, r);
    }

    void export_csv(std::string path) {
        std::ofstream file(path);
        for (int i = 0; i < num_steps_; ++i) {
            file << steps_[i].x() << "," << steps_[i].y() << std::endl;
        }
    }

private:
    int num_steps_;
    point *steps_;
    std::unordered_map<point, int, point_hash> occupied_;

    point pivot_point(int step, int i, rot r) {
        auto p = steps_[step];
        return p + r * (steps_[i] - p);
    }

    void set(int i, point p) {
        occupied_.erase(steps_[i]);
        steps_[i] = p;
        occupied_[p] = i;
    }

};

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <steps> <iters>" << std::endl;
        return 1;
    }

    int num_steps = std::stoi(argv[1]);
    int iters = std::stoi(argv[2]);
    walk w(num_steps);
    for (int i = 0; i < iters; ++i) {
        w.rand_pivot();
    }
    w.export_csv("walk.csv");
}
