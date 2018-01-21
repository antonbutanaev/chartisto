#include <gtest/gtest.h>
#include <chart/chart.h>

using namespace chart;

class EmptyBar: public data::Bar {
    time_t time(int) const override {return NoTime;}
    Price open(int) const override {return NoPrice;}
    Price close(int) const override {return NoPrice;}
    Price high(int) const override {return NoPrice;}
    Price low(int) const override {return NoPrice;}
    Volume volume(int) const override {return 0;}
};

class EmptyPoint: public data::Point {
public:
    time_t time(int) const override {return 0;}
    Price close(int) const override {return NoPrice;}
};

class EmptyBars: public data::Bars {
    size_t numBars() const override {return 0;}
    data::Bar &bar(size_t) const override {
        static EmptyBar bar;
        return bar;
    }
};

class EmptyPoints: public data::Points {
    size_t numPoints() const override {return 0;}
    data::Point &point(size_t) const override {
        static EmptyPoint point;
        return point;
    }
};

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
    EXPECT_NE(a, f);
}

TEST(TestChart, AddEmptyChart) {
	Canvas canvas;
	canvas.setCanvasSize({100, 100});
	EXPECT_EQ(canvas.numCharts(), 0);
	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 1);
	EXPECT_EQ(canvas.chart(0).h(), 100);
	EXPECT_EQ(canvas.chart(0).y(), 0);
}
