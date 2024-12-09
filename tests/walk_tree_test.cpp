#include <gtest/gtest.h>

#include "walk_node.h"
#include "walk_tree.h"

using namespace pivot;

TEST(WalkTreeInit, Line) {
    auto w = walk_tree<2>(5);
    ASSERT_TRUE(w.self_avoiding());
    auto steps = w.steps();
    EXPECT_EQ(steps.size(), 5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(steps[i], pivot::point<2>({i + 1, 0}));
    }
    EXPECT_FALSE(w.is_leaf());
    EXPECT_EQ(w.endpoint(), pivot::point<2>({5, 0}));
    for (int i = 1; i <= 4; i++) {
        auto &node = w.find_node(i);
        EXPECT_EQ(node.id(), i);
    }
}

TEST(WalkTreeInit, FromPoints2D) {
    // initialize a walk that isn't just a list of points
    pivot::walk_tree<2> w1(100);
    for (size_t i = 0; i < 100; ++i) {
        w1.rand_pivot();
    }

    // make sure outputs from steps is consistent with steps constructor
    auto steps1 = w1.steps();
    pivot::walk_tree<2> w2(steps1);
    auto steps2 = w2.steps();
    EXPECT_EQ(steps1.size(), steps2.size());
    EXPECT_EQ(steps1, steps2);
}

TEST(WalkTreeInit, FromPoints3D) {
    // initialize a walk that isn't just a list of points
    pivot::walk_tree<3> w1(100);
    for (size_t i = 0; i < 100; ++i) {
        w1.rand_pivot();
    }

    // make sure outputs from steps is consistent with steps constructor
    auto steps1 = w1.steps();
    pivot::walk_tree<3> w2(steps1);
    auto steps2 = w2.steps();
    EXPECT_EQ(steps1.size(), steps2.size());
    EXPECT_EQ(steps1, steps2);
}

TEST(WalkTreePivot, PivotLine) {
    auto w = walk_tree<2>(2);
    pivot::transform<2> trans = transform<2>({1, 0}, {-1, 1});
    EXPECT_TRUE(w.try_pivot(1, trans));
    auto steps = w.steps();
    EXPECT_EQ(steps.size(), 2);
    EXPECT_EQ(steps[0], pivot::point<2>({1, 0}));
    EXPECT_EQ(steps[1], pivot::point<2>({1, 1}));
}

TEST(WalkTreePivot, PivotLineFail) {
    auto w = walk_tree<2>(3);
    pivot::transform<2> trans = transform<2>({0, 1}, {-1, 1});
    EXPECT_FALSE(w.try_pivot(2, trans));
    auto steps = w.steps();
    EXPECT_EQ(steps.size(), 3);
    EXPECT_EQ(steps[0], pivot::point<2>({1, 0}));
    EXPECT_EQ(steps[1], pivot::point<2>({2, 0}));
    EXPECT_EQ(steps[2], pivot::point<2>({3, 0}));
}
