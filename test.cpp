#include "walk_tree.h"

int main() {
    auto w = pivot::walk_tree::line(10);
    w.rotate_left();
    w.rotate_left();
    w.rotate_left();
    w.rotate_right();
    w.todot("test.dot");
    return 0;
}
