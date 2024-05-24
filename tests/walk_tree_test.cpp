#include <gtest/gtest.h>

#include "walk_tree.h"

using namespace pivot;

TEST(WalkTreeInit, Line) {
    walk_tree *w = walk_tree::line(5);
    ASSERT_TRUE(w->self_avoiding());
    auto steps = w->steps();
    EXPECT_EQ(steps.size(), 5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(steps[i], pivot::point<2>({i + 1, 0}));
    }
    EXPECT_FALSE(w->is_leaf());
    EXPECT_EQ(w->endpoint(), pivot::point<2>({5, 0}));
    delete w;
}

TEST(WalkTreePivot, PivotLine) {
    walk_tree *w = walk_tree::line(2);
    EXPECT_TRUE(w->try_pivot(1, angle::ninety));
    auto steps = w->steps();
    EXPECT_EQ(steps.size(), 2);
    EXPECT_EQ(steps[0], pivot::point<2>({1, 0}));
    EXPECT_EQ(steps[1], pivot::point<2>({1, 1}));
    delete w;
}

TEST(WalkTreePivot, PivotLineFail) {
    walk_tree *w = walk_tree::line(3);
    EXPECT_FALSE(w->try_pivot(2, angle::one_eighty));
    auto steps = w->steps();
    EXPECT_EQ(steps.size(), 3);
    EXPECT_EQ(steps[0], pivot::point<2>({1, 0}));
    EXPECT_EQ(steps[1], pivot::point<2>({2, 0}));
    EXPECT_EQ(steps[2], pivot::point<2>({3, 0}));
    delete w;
}
