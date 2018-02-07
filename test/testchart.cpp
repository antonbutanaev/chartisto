#include <gtest/gtest.h>
#include <chart/chart.h>

using namespace chart;

class EmptyBar: public data::Bar {
    Time time() const override {return NoTime;}
    Price open() const override {return NoPrice;}
    Price close() const override {return NoPrice;}
    Price high() const override {return NoPrice;}
    Price low() const override {return NoPrice;}
    Volume volume() const override {return 0;}
};

class EmptyPoint: public data::Point {
public:
    Time time(int) const override {return NoTime;}
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

TEST(TestChart, AddCharts) {
	Canvas canvas;
	canvas.setCanvasSize({100, 100});
	EXPECT_EQ(canvas.numCharts(), 0);
	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 1);
	EXPECT_EQ(canvas.chart(0).h(), 100);

	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 2);
	EXPECT_EQ(canvas.chart(0).h(), 75);
	EXPECT_EQ(canvas.chart(1).h(), 25);

	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 3);
	EXPECT_EQ(canvas.chart(0).h(), 75*3/4);
	EXPECT_EQ(canvas.chart(1).h(), 25*3/4);
	EXPECT_EQ(canvas.chart(2).h(), 25);
}

TEST(TestChart, ResizeCharts) {
	Canvas canvas;
	canvas.setCanvasSize({100, 100});
	EXPECT_EQ(canvas.numCharts(), 0);
	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 1);
	EXPECT_EQ(canvas.chart(0).h(), 100);

	canvas.addChart(Chart());
	EXPECT_EQ(canvas.numCharts(), 2);
	EXPECT_EQ(canvas.chart(0).h(), 75);
	EXPECT_EQ(canvas.chart(1).h(), 25);

	canvas.setCanvasSize({200, 200});
	EXPECT_EQ(canvas.numCharts(), 2);
	EXPECT_EQ(canvas.chart(0).h(), 2*75);
	EXPECT_EQ(canvas.chart(1).h(), 2*25);

}
