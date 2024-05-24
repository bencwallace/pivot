#include <gtest/gtest.h>

#include "utils.h"

using namespace pivot;

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
