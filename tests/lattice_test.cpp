#include <array>

#include <gtest/gtest.h>

#include "lattice.h"

using namespace pivot;

TEST(PointTest, ToString) {
    auto p = point({1, 2});
    EXPECT_EQ(p.to_string(), "(1, 2)");
    auto q = point({3,});
    EXPECT_EQ(q.to_string(), "(3,)");
}

TEST(BoxTest, FromSpan1D) {
    auto p1 = point({3});
    auto b1 = box(std::array{p1});
    EXPECT_EQ(b1.intervals_[0].left_, 1);
    EXPECT_EQ(b1.intervals_[0].right_, 1);

    auto p2 = point({1});
    auto b2 = box(std::array{p1, p2});
    EXPECT_EQ(b2.intervals_[0].left_, -1);
    EXPECT_EQ(b2.intervals_[0].right_, 1);

    auto b3 = box(std::array{p2, p1});
    EXPECT_EQ(b3.intervals_[0].left_, 1);
    EXPECT_EQ(b3.intervals_[0].right_, 3);

    auto p3 = point({-2});
    auto b4 = box(std::array{p1, p2, p3});
    EXPECT_EQ(b4.intervals_[0].left_, -4);
    EXPECT_EQ(b4.intervals_[0].right_, 1);

    auto b5 = box(std::array{p3, p2, p1});
    EXPECT_EQ(b5.intervals_[0].left_, 1);
    EXPECT_EQ(b5.intervals_[0].right_, 6);

    auto p4 = point({0});
    auto b6 = box(std::array{p1, p2, p3, p4});
    EXPECT_EQ(b6.intervals_[0].left_, -4);
    EXPECT_EQ(b6.intervals_[0].right_, 1);
}

TEST(BoxTest, FromSpan2D) {
    auto p1 = point({1, 2});
    auto b1 = box(std::array{p1});
    EXPECT_EQ(b1.intervals_[0].left_, 1);
    EXPECT_EQ(b1.intervals_[0].right_, 1);
    EXPECT_EQ(b1.intervals_[1].left_, 0);
    EXPECT_EQ(b1.intervals_[1].right_, 0);

    auto p2 = point({5, 5});
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

    auto p3 = point({3, 4});
    auto b4 = box(std::array{p1, p2, p3});
    EXPECT_EQ(b4.intervals_[0].left_, 1);
    EXPECT_EQ(b4.intervals_[0].right_, 5);
    EXPECT_EQ(b4.intervals_[1].left_, 0);
    EXPECT_EQ(b4.intervals_[1].right_, 3);
}

TEST(BoxTest, Union1D) {
    box b1({interval{-1, 1}});
    box b2({interval{0, 2}});
    auto b3 = b1 | b2;
    EXPECT_EQ(b3.intervals_[0].left_, -1);
    EXPECT_EQ(b3.intervals_[0].right_, 2);

    box b4({interval{2, 3}});
    auto b5 = b1 | b4;
    EXPECT_EQ(b5.intervals_[0].left_, -1);
    EXPECT_EQ(b5.intervals_[0].right_, 3);

    box b6({interval{0, 1}});
    auto b7 = b1 | b6;
    EXPECT_EQ(b7.intervals_[0].left_, -1);
    EXPECT_EQ(b7.intervals_[0].right_, 1);
}

TEST(BoxTest, Union2D) {
    box b1({interval{-1, 1}, interval{0, 1}});
    box b2({interval{0, 2}, interval{0, 2}});
    auto b3 = b1 | b2;
    EXPECT_EQ(b3.intervals_[0].left_, -1);
    EXPECT_EQ(b3.intervals_[0].right_, 2);
    EXPECT_EQ(b3.intervals_[1].left_, 0);
    EXPECT_EQ(b3.intervals_[1].right_, 2);

    box b4({interval{2, 3}, interval{1, 2}});
    auto b5 = b1 | b4;
    EXPECT_EQ(b5.intervals_[0].left_, -1);
    EXPECT_EQ(b5.intervals_[0].right_, 3);
    EXPECT_EQ(b5.intervals_[1].left_, 0);
    EXPECT_EQ(b5.intervals_[1].right_, 2);
}

TEST(BoxTest, Intersection1D) {
    box b1({interval{-1, 1}});
    box b2({interval{0, 2}});
    auto b3 = b1 & b2;
    EXPECT_EQ(b3.intervals_[0].left_, 0);
    EXPECT_EQ(b3.intervals_[0].right_, 1);

    box b4({interval{2, 3}});
    auto b5 = b1 & b4;
    EXPECT_TRUE(b5.empty());

    box b6({interval{1, 2}});
    auto b7 = b1 & b6;
    EXPECT_EQ(b7.intervals_[0].left_, 1);
    EXPECT_EQ(b7.intervals_[0].right_, 1);
}

TEST(BoxTest, Intersection2D) {
    box b1({interval{-1, 1}, interval{0, 1}});
    box b2({interval{0, 2}, interval{0, 2}});
    auto b3 = b1 & b2;
    EXPECT_EQ(b3.intervals_[0].left_, 0);
    EXPECT_EQ(b3.intervals_[0].right_, 1);
    EXPECT_EQ(b3.intervals_[1].left_, 0);
    EXPECT_EQ(b3.intervals_[1].right_, 1);

    box b4({interval{2, 3}, interval{1, 2}});
    auto b5 = b1 & b4;
    EXPECT_TRUE(b5.empty());
}

TEST(BoxTest, ToString) {
    box b1({interval{-1, 1}});
    EXPECT_EQ(b1.to_string(), "[-1, 1]");

    box b2({interval{-1, 1}, interval{0, 1}});
    EXPECT_EQ(b2.to_string(), "[-1, 1] x [0, 1]");
}

TEST(TransformTest, Pivot2D) {
    auto e0 = point::unit(2, 0);

    point p1({3, 4});
    point p2({4, 4});
    transform<2> t1(p1, p2);
    EXPECT_EQ(p1 + t1 * e0, p2);

    point p3({3, 5});
    transform<2> t2(p1, p3);
    EXPECT_EQ(p1 + t2 * e0, p3);
}

TEST(TransformTest, Pivot3D) {
    auto e0 = point::unit(3, 0);

    point p1({3, 4, 5});
    point p2({4, 4, 5});
    transform<3> t1(p1, p2);
    EXPECT_EQ(p1 + t1 * e0, p2);

    point p3({3, 5, 5});
    transform<3> t2(p1, p3);
    EXPECT_EQ(p1 + t2 * e0, p3);
}

TEST(TransformTest, Compose2D) {
    auto t1 = transform<2>::rand();
    auto t2 = transform<2>::rand();
    auto t3 = t1 * t2;
    auto p = point({1, 2});
    EXPECT_EQ(t3 * p, t1 * (t2 * p)) << "t1: " << t1.to_string() << ", t2: " << t2.to_string();
}

TEST(TransformTest, Compose3D) {
    auto t1 = transform<3>::rand();
    auto t2 = transform<3>::rand();
    auto t3 = t1 * t2;
    auto p = point({1, 2, 3});
    EXPECT_EQ(t3 * p, t1 * (t2 * p)) << "t1: " << t1.to_string() << ", t2: " << t2.to_string();
}

TEST(TransformTest, Box2D) {
    box b({interval{1, 5}, interval{2, 4}});

    point p1({0, 0});
    point p2({0, 1});
    transform<2> t1(p1, p2);
    EXPECT_EQ(t1 * b, box({interval{-4, -2}, interval{1, 5}}));

    std::array perm = {0, 1};
    std::array signs = {-1, 1};
    transform<2> t2(perm, signs);
    EXPECT_EQ(t2 * b, box({interval{-5, -1}, interval{2, 4}}));
}

TEST(TransformTest, Box3D) {
    box b({interval{1, 5}, interval{2, 4}, interval{3, 6}});

    point p1({0, 0, 0});
    point p2({0, 1, 0});
    transform<3> t1(p1, p2);
    EXPECT_EQ(t1 * b, box({interval{-4, -2}, interval{1, 5}, interval{3, 6}}));

    transform<3> t2(std::array{0, 1, 2}, std::array{-1, 1, 1});
    EXPECT_EQ(t2 * b, box({interval{-5, -1}, interval{2, 4}, interval{3, 6}}));

    transform<3> t3(std::array{2, 0, 1}, std::array{1, 1, 1});
    EXPECT_EQ(t3 * b, box({interval{2, 4}, interval{3, 6}, interval{1, 5}}));
}

TEST(TransformTest, Inverse2D) {
    transform<2> id{};
    auto o = point({0, 0});
    auto e1 = point({1, 0});
    auto e2 = point({0, 1});
    ASSERT_EQ(e1, id * e1);
    ASSERT_EQ(e2, id * e2);

    std::array perm = {1, 0};
    std::array signs = {-1, 1};
    transform<2> t(perm, signs);
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

TEST(TransformTest, Inverse3D) {
    transform<3> id{};
    auto e1 = point({1, 0, 0});
    auto e2 = point({0, 1, 0});
    auto e3 = point({0, 0, 1});
    ASSERT_EQ(e1, id * e1);
    ASSERT_EQ(e2, id * e2);
    ASSERT_EQ(e3, id * e3);

    transform<3> t(std::array{2, 0, 1}, std::array{-1, 1, -1});
    transform<3> t_inv = t.inverse();
    auto f1 = t * e1;
    auto f2 = t * e2;
    auto f3 = t * e3;
    EXPECT_EQ(e1, t_inv * f1);
    EXPECT_EQ(e2, t_inv * f2);
    EXPECT_EQ(e3, t_inv * f3);
}
