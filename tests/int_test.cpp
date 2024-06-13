#include <gtest/gtest.h>

#include "walk.h"
#include "walk_tree.h"

using namespace pivot;

TEST(WalkTest, SelfAvoiding) {
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    walk<2> w(100);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 100; j++) {
        w.rand_pivot();
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}

TEST(WalkTreeTest, SelfAvoiding) {
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    auto w = walk_tree<2>(2);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 100; j++) {
        w.rand_pivot();
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}
