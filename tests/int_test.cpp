#include <gtest/gtest.h>

#include "walk.h"
#include "walk_tree.h"

using namespace pivot;

TEST(IntTest, Walk) {
  walk w(100);
  for (int i = 0; i < 10; ++i) {
    for (int i = 0; i < 100; i++) {
      w.rand_pivot();
    }
    EXPECT_TRUE(w.self_avoiding());
  }
}

TEST(IntTest, WalkTree) {
  walk_tree *w = walk_tree::line(100);
  for (int i = 0; i < 10; ++i) {
    for (int i = 0; i < 100; i++) {
      w->rand_pivot();
    }
    EXPECT_TRUE(w->self_avoiding());
  }
}
