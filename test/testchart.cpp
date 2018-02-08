#include <gtest/gtest.h>
#include <chart/chart.h>
#include <chart/ema.h>
#include <date.h>

using namespace chart;
using namespace std;
using namespace date;

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

TEST(TestChart, EMA) {
	class Points: public data::Points {
	    std::string title() const override {return "TestEma";}
	    Time time(size_t n) const override {return sys_days{2017_y/2/(n+1)};}
	    Price close(size_t n) const override {return n+1;}
	    size_t numPoints() const override {return 3;}
	};

	const auto emaPoints = ema(make_shared<Points>(), 3);
	ASSERT_EQ(emaPoints->numPoints(), 3);
	EXPECT_EQ(emaPoints->title(), "TestEma EMA 3");

	const auto k = 2. / (1 + 3);
	const auto ema1 = 1;

	EXPECT_EQ(emaPoints->time(0), sys_days{2017_y/2/1});
	EXPECT_EQ(emaPoints->close(0), ema1);

	const auto ema2 = 2 * k + ema1 * (1-k);
	EXPECT_EQ(emaPoints->time(1), sys_days{2017_y/2/2});
	EXPECT_EQ(emaPoints->close(1), ema2);

	const auto ema3 = 3 * k + ema2 * (1-k);
	EXPECT_EQ(emaPoints->time(2), sys_days{2017_y/2/3});
	EXPECT_EQ(emaPoints->close(2), ema3);
}
