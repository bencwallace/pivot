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
    expect_box = pivot::box<2>(std::array{interval{1, 1}, interval{-1, 0}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {1, -1}));
    EXPECT_EQ(end, pivot::point<2>({1, -1}));
    EXPECT_EQ(b, expect_box) << "b: " << b.to_string() << " expect_box: " << expect_box.to_string();

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

TEST(WalkNode, Balanced3) {
    std::vector steps = {pivot::point<2>({1, 0}), pivot::point<2>({2, 0}), pivot::point<2>({2, 1}), pivot::point<2>({2, 2})};
    auto root = walk_node<2>::balanced_rep(steps);

    auto symm = root->symm();
    auto end = root->endpoint();
    auto b = root->bbox();
    auto expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 2}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {-1, 1}));
    EXPECT_EQ(end, pivot::point<2>({2, 2}));
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
    EXPECT_EQ(symm, transform<2>());
    EXPECT_EQ(end, pivot::point<2>({2, 0}));
    EXPECT_EQ(b, expect_box);

    EXPECT_TRUE(left->left()->is_leaf());
    EXPECT_TRUE(left->right()->is_leaf());
    EXPECT_TRUE(right->left()->is_leaf());
    EXPECT_TRUE(right->right()->is_leaf());

    delete root;
}

TEST(RandomWalk, IsNearestNeighbor) {
    auto steps = random_walk<2>(100);
    ASSERT_EQ(steps.size(), 100);
    for (int i = 1; i < 100; ++i) {
        EXPECT_EQ((steps[i] - steps[i - 1]).norm(), 1);
    }
}

TEST(WalkNode, BalancedSteps) {
    auto steps = random_walk<2>(100);
    auto root = walk_node<2>::balanced_rep(steps);
    auto result = root->steps();
    EXPECT_EQ(steps, result);
    delete root;
}

TEST(WalkNode, RotateRight2D1) {
    std::vector steps = {pivot::point<2>({1, 0}), pivot::point<2>({2, 0}), pivot::point<2>({3, 0})};
    auto root = walk_node<2>::balanced_rep(steps);

    root->rotate_right();
    auto symm = root->symm();
    auto end = root->endpoint();
    auto b = root->bbox();
    auto expect_box = pivot::box<2>(std::array{interval{1, 3}, interval{0, 0}});
    EXPECT_EQ(symm, transform<2>());
    EXPECT_EQ(end, pivot::point<2>({3, 0}));
    EXPECT_EQ(b, expect_box);

    auto right = root->right();
    symm = right->symm();
    end = right->endpoint();
    b = right->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 0}});
    EXPECT_EQ(symm, transform<2>());
    EXPECT_EQ(end, pivot::point<2>({2, 0}));
    EXPECT_EQ(b, expect_box);

    auto left = root->left();
    EXPECT_TRUE(left->is_leaf());
    EXPECT_TRUE(right->left()->is_leaf());
    EXPECT_TRUE(right->right()->is_leaf());

    delete root;
}

TEST(WalkNode, RotateRight2D2) {
    std::vector steps = {pivot::point<2>({1, 0}), pivot::point<2>({1, 1}), pivot::point<2>({1, 2})};
    auto root = walk_node<2>::balanced_rep(steps);

    root->rotate_right();
    auto symm = root->symm();
    auto end = root->endpoint();
    auto b = root->bbox();
    auto expect_box = pivot::box<2>(std::array{interval{1, 1}, interval{0, 2}});
    EXPECT_EQ(symm, transform<2>({1, 0}, {-1, 1}));
    EXPECT_EQ(end, pivot::point<2>({1, 2}));
    EXPECT_EQ(b, expect_box);

    auto right = root->right();
    symm = right->symm();
    end = right->endpoint();
    b = right->bbox();
    expect_box = pivot::box<2>(std::array{interval{1, 2}, interval{0, 0}});
    EXPECT_EQ(symm, transform<2>());
    EXPECT_EQ(end, pivot::point<2>({2, 0}));
    EXPECT_EQ(b, expect_box);

    auto left = root->left();
    EXPECT_TRUE(left->is_leaf());
    EXPECT_TRUE(right->left()->is_leaf());
    EXPECT_TRUE(right->right()->is_leaf());

    delete root;
}

TEST(WalkNode, RotateRightStepsRand2D) {
    int num_sites = 100;
    auto steps = random_walk<2>(num_sites);

    auto root = walk_node<2>::balanced_rep(steps);
    ASSERT_EQ(root->steps(), steps);
    EXPECT_EQ(root->rotate_right()->steps(), steps);
    delete root;
}

TEST(WalkNode, RotateLeftStepsRand2D) {
    int num_sites = 100;
    auto steps = random_walk<2>(num_sites);

    auto root = walk_node<2>::balanced_rep(steps);
    ASSERT_EQ(root->steps(), steps);
    EXPECT_EQ(root->rotate_left()->steps(), steps);
    delete root;
}

TEST(WalkNode, RotateLeftRightRand2D) {
    int num_sites = 100;
    auto steps = random_walk<2>(num_sites);

    auto root1 = walk_node<2>::balanced_rep(steps);
    auto root2 = walk_node<2>::balanced_rep(steps);
    EXPECT_EQ(*root1->rotate_left()->rotate_right(), *root2);
    delete root1;
    delete root2;
}

TEST(WalkNode, RotateRightLeftRand2D) {
    int num_sites = 100;
    auto steps = random_walk<2>(num_sites);

    auto root1 = walk_node<2>::balanced_rep(steps);
    auto root2 = walk_node<2>::balanced_rep(steps);
    EXPECT_EQ(*root1->rotate_right()->rotate_left(), *root2);
    delete root1;
    delete root2;
}
