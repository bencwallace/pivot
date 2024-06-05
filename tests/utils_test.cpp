#include <array>

#include <gtest/gtest.h>

#include "utils.h"

using namespace pivot;

TEST(PointTest, ToString) {
    auto p = point<2>({1, 2});
    EXPECT_EQ(p.to_string(), "(1, 2)");
    auto q = point<1>({3});
    EXPECT_EQ(q.to_string(), "(3,)");
}

TEST(BoxTest, FromSpan1D) {
    auto p1 = point<1>({3});
    auto b1 = box(std::array{p1});
    EXPECT_EQ(b1.intervals_[0].left_, 1);
    EXPECT_EQ(b1.intervals_[0].right_, 1);

    auto p2 = point<1>({1});
    auto b2 = box(std::array{p1, p2});
    EXPECT_EQ(b2.intervals_[0].left_, -1);
    EXPECT_EQ(b2.intervals_[0].right_, 1);

    auto b3 = box(std::array{p2, p1});
    EXPECT_EQ(b3.intervals_[0].left_, 1);
    EXPECT_EQ(b3.intervals_[0].right_, 3);

    auto p3 = point<1>({-2});
    auto b4 = box(std::array{p1, p2, p3});
    EXPECT_EQ(b4.intervals_[0].left_, -4);
    EXPECT_EQ(b4.intervals_[0].right_, 1);

    auto b5 = box(std::array{p3, p2, p1});
    EXPECT_EQ(b5.intervals_[0].left_, 1);
    EXPECT_EQ(b5.intervals_[0].right_, 6);

    auto p4 = point<1>({0});
    auto b6 = box(std::array{p1, p2, p3, p4});
    EXPECT_EQ(b6.intervals_[0].left_, -4);
    EXPECT_EQ(b6.intervals_[0].right_, 1);
}

TEST(BoxTest, FromSpan2D) {
    auto p1 = point<2>({1, 2});
    auto b1 = box(std::array{p1});
    EXPECT_EQ(b1.intervals_[0].left_, 1);
    EXPECT_EQ(b1.intervals_[0].right_, 1);
    EXPECT_EQ(b1.intervals_[1].left_, 0);
    EXPECT_EQ(b1.intervals_[1].right_, 0);

    auto p2 = point<2>({5, 5});
    auto b2 = box(std::array{p1, p2});
    EXPECT_EQ(b2.intervals_[0].left_, 1);
    EXPECT_EQ(b2.intervals_[0].right_, 5);
    EXPECT_EQ(b2.intervals_[1].left_, 0);
    EXPECT_EQ(b2.intervals_[1].right_, 3);

    auto b3 = box(std::array{p2, p1});
    EXPECT_EQ(b3.intervals_[0].left_, -3);
    EXPECT_EQ(b3.intervals_[0].right_, 1);
    EXPECT_EQ(b3.intervals_[1].left_, -3);
    EXPECT_EQ(b3.intervals_[1].right_, 0);

    auto p3 = point<2>({3, 4});
    auto b4 = box(std::array{p1, p2, p3});
    EXPECT_EQ(b4.intervals_[0].left_, 1);
    EXPECT_EQ(b4.intervals_[0].right_, 5);
    EXPECT_EQ(b4.intervals_[1].left_, 0);
    EXPECT_EQ(b4.intervals_[1].right_, 3);
}

TEST(BoxTest, Union1D) {
    box<1> b1({interval{-1, 1}});
    box<1> b2({interval{0, 2}});
    auto b3 = b1 + b2;
    EXPECT_EQ(b3.intervals_[0].left_, -1);
    EXPECT_EQ(b3.intervals_[0].right_, 2);

    box<1> b4({interval{2, 3}});
    auto b5 = b1 + b4;
    EXPECT_EQ(b5.intervals_[0].left_, -1);
    EXPECT_EQ(b5.intervals_[0].right_, 3);

    box<1> b6({interval{0, 1}});
    auto b7 = b1 + b6;
    EXPECT_EQ(b7.intervals_[0].left_, -1);
    EXPECT_EQ(b7.intervals_[0].right_, 1);
}

TEST(BoxTest, Union2D) {
    box<2> b1({interval{-1, 1}, interval{0, 1}});
    box<2> b2({interval{0, 2}, interval{0, 2}});
    auto b3 = b1 + b2;
    EXPECT_EQ(b3.intervals_[0].left_, -1);
    EXPECT_EQ(b3.intervals_[0].right_, 2);
    EXPECT_EQ(b3.intervals_[1].left_, 0);
    EXPECT_EQ(b3.intervals_[1].right_, 2);

    box<2> b4({interval{2, 3}, interval{1, 2}});
    auto b5 = b1 + b4;
    EXPECT_EQ(b5.intervals_[0].left_, -1);
    EXPECT_EQ(b5.intervals_[0].right_, 3);
    EXPECT_EQ(b5.intervals_[1].left_, 0);
    EXPECT_EQ(b5.intervals_[1].right_, 2);
}

TEST(BoxTest, Intersection1D) {
    box<1> b1({interval{-1, 1}});
    box<1> b2({interval{0, 2}});
    auto b3 = b1 * b2;
    EXPECT_EQ(b3.intervals_[0].left_, 0);
    EXPECT_EQ(b3.intervals_[0].right_, 1);

    box<1> b4({interval{2, 3}});
    auto b5 = b1 * b4;
    EXPECT_TRUE(b5.empty());

    box<1> b6({interval{1, 2}});
    auto b7 = b1 * b6;
    EXPECT_EQ(b7.intervals_[0].left_, 1);
    EXPECT_EQ(b7.intervals_[0].right_, 1);
}

TEST(BoxTest, Intersection2D) {
    box<2> b1({interval{-1, 1}, interval{0, 1}});
    box<2> b2({interval{0, 2}, interval{0, 2}});
    auto b3 = b1 * b2;
    EXPECT_EQ(b3.intervals_[0].left_, 0);
    EXPECT_EQ(b3.intervals_[0].right_, 1);
    EXPECT_EQ(b3.intervals_[1].left_, 0);
    EXPECT_EQ(b3.intervals_[1].right_, 1);

    box<2> b4({interval{2, 3}, interval{1, 2}});
    auto b5 = b1 * b4;
    EXPECT_TRUE(b5.empty());
}

TEST(BoxTest, ToString) {
    box<1> b1({interval{-1, 1}});
    EXPECT_EQ(b1.to_string(), "[-1, 1]");

    box<2> b2({interval{-1, 1}, interval{0, 1}});
    EXPECT_EQ(b2.to_string(), "[-1, 1] x [0, 1]");
}

TEST(TransformTest, Inverse2D) {
    transform<2> id({0, 1}, {1, 1});
    auto o = point<2>({0, 0});
    auto e1 = point<2>({1, 0});
    auto e2 = point<2>({0, 1});
    ASSERT_EQ(e1, id * e1);
    ASSERT_EQ(e2, id * e2);

    transform<2> t({1, 0}, {-1, 1});
    transform<2> t_inv = t.inverse();
    auto f1 = t * e1;
    auto f2 = t * e2;
    ASSERT_EQ(f1, e2);
    ASSERT_EQ(f2, o - e1);
    EXPECT_EQ(e1, t_inv * f1);
    EXPECT_EQ(e2, t_inv * f2);

    auto ttinv = t * t_inv;
    auto tinvt = t_inv * t;
    EXPECT_EQ(ttinv, id);
    EXPECT_EQ(tinvt, id);
}
