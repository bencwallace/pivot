#include <vector>

#include <gtest/gtest.h>

#include "walk_tree.h"

#include "test_utils.h"

using namespace pivot;

TEST(WalkNode, Balanced1) {
    auto steps = {pivot::point<2>({1, 0}), pivot::point<2>({2, 0}), pivot::point<2>({2, 1}), pivot::point<2>({3, 1})};
    auto root = walk_node<2>::balanced_rep(steps);

    auto symm = root->symm();
    auto end = root->endpoint();
    auto b = root->bbox();
    auto expect_box = pivot::box<2>(std::array{interval{1, 3}, interval{0, 1}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {-1, 1}));
    EXPECT_EQ(end, pivot::point<2>({3, 1}));
    EXPECT_EQ(b, expect_box);

    auto left = root->left();
    symm = left->symm();
    end = left->endpoint();
    b = left->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 0}});
    EXPECT_EQ(symm, transform<2>());
    EXPECT_EQ(end, pivot::point<2>({2, 0}));
    EXPECT_EQ(b, expect_box);

    auto right  = root->right();
    symm = right->symm();
    end = right->endpoint();
    b = right->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 0}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {1, -1}));
    EXPECT_EQ(end, pivot::point<2>({1, -1}));
    EXPECT_EQ(b, expect_box);

    EXPECT_TRUE(left->left()->is_leaf());
    EXPECT_TRUE(left->right()->is_leaf());
    EXPECT_TRUE(right->left()->is_leaf());
    EXPECT_TRUE(right->right()->is_leaf());

    delete root;
}

TEST(WalkNode, Balanced2) {
    // steps and tree from Clisby (2010), Figs. 1, 23
    auto steps = {pivot::point<2>({1, 0}), pivot::point<2>({1, 1}), pivot::point<2>({2, 1}), pivot::point<2>({3, 1}),
                  pivot::point<2>({3, 0})};
    auto root = walk_node<2>::balanced_rep(steps);

    auto symm = root->symm();
    auto end = root->endpoint();
    auto b = root->bbox();
    auto expect_box = pivot::box<2>(std::array{interval{1, 3}, interval{0, 1}});
    EXPECT_EQ(symm, transform<2>({0, 1}, {1, 1}));
    EXPECT_EQ(end, pivot::point<2>({3, 0}));
    EXPECT_EQ(b, expect_box);

    auto left = root->left();
    symm = left->symm();
    end = left->endpoint();
    b = left->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 1}});
    EXPECT_EQ(symm, transform<2>({0, 1}, {1, 1}));
    EXPECT_EQ(end, pivot::point<2>({2, 1}));
    EXPECT_EQ(b, expect_box);

    auto right  = root->right();
    symm = right->symm();
    end = right->endpoint();
    b = right->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 1}, interval{-1, 0}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {1, -1}));
    EXPECT_EQ(end, pivot::point<2>({1, -1}));
    EXPECT_EQ(b, expect_box);

    symm = left->left()->symm();
    end = left->left()->endpoint();
    b = left->left()->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 1}, interval{0, 1}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {-1, 1}));
    EXPECT_EQ(end, pivot::point<2>({1, 1}));
    EXPECT_EQ(b, expect_box);

    EXPECT_TRUE(left->right()->is_leaf());
    EXPECT_TRUE(right->left()->is_leaf());
    EXPECT_TRUE(right->right()->is_leaf());
    EXPECT_TRUE(left->left()->left()->is_leaf());
    EXPECT_TRUE(left->left()->right()->is_leaf());

    delete root;
}

TEST(WalkNode, BalancedSteps) {
    auto steps = random_walk<2>(100);
    auto root = walk_node<2>::balanced_rep(steps);
    auto result = root->steps();
    EXPECT_EQ(steps, result);
    delete root;
}
