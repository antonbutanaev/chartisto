#include <gtest/gtest.h>
#include <chart/chart.h>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <robotrade/quotesParser.h>
#include <date/date.h>

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
