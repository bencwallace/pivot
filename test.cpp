#include "walk_tree.h"

int main() {
    auto w = pivot::walk_tree::line(4);
    w.todot("test.dot");
    return 0;
}
