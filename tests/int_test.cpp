#include <gtest/gtest.h>

#include "walk.h"
#include "walk_tree.h"

using namespace pivot;

TEST(IntTest, Walk) {
  walk<2> w(100);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 100; j++) {
      w.rand_pivot();
    }
    EXPECT_TRUE(w.self_avoiding());
  }
}

TEST(IntTest, WalkTree) {
  walk_tree<2> *w = walk_tree<2>::line(2);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 100; j++) {
      w->rand_pivot();
    }
    EXPECT_TRUE(w->self_avoiding());
  }
  delete w;
}
