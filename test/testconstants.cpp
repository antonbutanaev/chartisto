#include <gtest/gtest.h>
#include <chart/chart.h>

using namespace chart;

TEST(TestChart, TestConstants) {
    const auto a = NoPrice;
    const auto b = NoPrice;
    EXPECT_EQ(a, b);
    const Price c = 0;
    EXPECT_NE(a, c);

    const auto d = NoTime;
    const auto e = NoTime;
    EXPECT_EQ(d, e);
    const Time f = 0;
    EXPECT_NE(d, f);
}

