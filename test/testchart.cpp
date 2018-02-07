#include <gtest/gtest.h>
#include <chart/chart.h>

using namespace chart;

class EmptyBars: public data::Bars {
    Time time(size_t) const override {return NoTime;}
    Price open(size_t) const override {return NoPrice;}
    Price close(size_t) const override {return NoPrice;}
    Price high(size_t) const override {return NoPrice;}
    Price low(size_t) const override {return NoPrice;}
    Volume volume(size_t) const override {return 0;}
    size_t numBars() const override {return 0;}
};

class EmptyPoints: public data::Points {
    Time time(size_t) const override {return NoTime;}
    Price close(size_t) const override {return NoPrice;}
    size_t numPoints() const override {return 0;}
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
