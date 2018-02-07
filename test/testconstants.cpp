#include <chrono>
#include <gtest/gtest.h>
#include <chart/chart.h>
#include <date.h>

using namespace chart;
using namespace std;
using namespace std::chrono;
using namespace date;

TEST(TestChart, TestConstants) {
    const auto a = NoPrice;
    const auto b = NoPrice;
    EXPECT_EQ(a, b);
    const Price c = 0;
    EXPECT_NE(a, c);

    const auto d = NoTime;
    const auto e = NoTime;
    EXPECT_EQ(d, e);

    const Time f = sys_days{2017_y/feb/7} + 10h + 20min;
    EXPECT_NE(d, f);
}

