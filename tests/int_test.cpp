#include <random>

#include <gtest/gtest.h>

#include "loop.h"
#include "walk.h"
#include "walk_node.h"
#include "walk_tree.h"

using namespace pivot;

TEST(WalkTest, SelfAvoiding) {
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    walk<2> w(num_steps);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; j++) {
        w.rand_pivot();
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}

TEST(WalkTest, SelfAvoiding3D) {
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    walk<3> w(num_steps);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; j++) {
        w.rand_pivot();
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}

TEST(WalkTest, Seed) {
  std::random_device rd;
  auto seed = rd();
  std::mt19937 gen(seed);

  walk<2> w1(100, seed);
  walk<2> w2(100, seed);
  for (int i = 0; i < 10; ++i) {
    w1.rand_pivot();
    w2.rand_pivot();
  }
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(w1[i], w2[i]);
  }
}

TEST(WalkTest, Loop) {
  auto ret = main_loop<2>(100, 10, true, false, 42, false, false, "", "");
  ASSERT_EQ(ret, 0);
}

TEST(WalkTreeTest, SelfAvoiding) {
  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(0, 1);
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    auto w = walk_tree<2>(num_steps);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; j++) {
        w.rand_pivot(static_cast<bool>(dist(gen)));
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}

TEST(WalkTreeTest, SelfAvoiding3D) {
  for (int num_steps = 2; num_steps < 10; ++num_steps) {
    auto w = walk_tree<3>(num_steps);
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; j++) {
        w.rand_pivot();
      }
      EXPECT_TRUE(w.self_avoiding());
    }
  }
}

TEST(WalkTreeTest, FindNode) {
  auto w = walk_tree<2>(100);
  for (int i = 1; i <= 100; ++i) {
    w.rand_pivot();
  }
  for (int i = 1; i < 100; ++i) {
    auto &node = w.find_node(i);
    ASSERT_EQ(node.id(), i);
  }
}

TEST(WalkTreeTest, Seed) {
  std::random_device rd;
  auto seed = rd();
  std::mt19937 gen(seed);

  walk_tree<2> w1(100, seed);
  walk_tree<2> w2(100, seed);
  walk_tree<2> w3(100);
  std::srand(seed);
  for (int i = 0; i < 10; ++i) {
    w1.rand_pivot();
    w2.rand_pivot();
    w3.rand_pivot();
  }
  auto steps1 = w1.steps();
  auto steps2 = w2.steps();
  auto steps3 = w3.steps();
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(steps1[i], steps2[i]);
  }
  ASSERT_NE(steps1, steps3);
}

TEST(WalkTreeTest, Loop) {
  auto ret = main_loop<2>(100, 10, false, true, 42, false, false, "", "");
  ASSERT_EQ(ret, 0);
}
