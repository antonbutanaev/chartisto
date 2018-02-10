#include <gtest/gtest.h>
#include <chart/chart.h>
#include <chart/ema.h>
#include <chart/macd.h>
#include <chart/reduce.h>
#include <robotrade/quotesparser.h>
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
		size_t num() const override {return 3;}
	};

	const auto emaPoints = ema(make_shared<Points>(), 3);
	ASSERT_EQ(emaPoints->num(), 3);
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

TEST(TestChart, Reduce) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180208,100000,1,10,1,4,5\n"
		"VTBR,D,20180209,110000,6,11,6,9,10\n"
		"VTBR,D,20180212,120000,11,20,10,14,15\n"
		"VTBR,D,20180213,130000,16,22,11,19,20\n"
		;
	stringstream ss(quotes);

	const auto bars = robotrade::parse(ss);

	ASSERT_EQ(bars->num(), 4);

	const auto barsWeek = reduce(*bars, weekReduce);
	ASSERT_EQ(barsWeek->num(), 2);

	EXPECT_EQ(barsWeek->time(0), sys_days{2018_y/2/5});
	EXPECT_EQ(barsWeek->volume(0), 15);
	EXPECT_EQ(barsWeek->open(0), 1);
	EXPECT_EQ(barsWeek->close(0), 9);
	EXPECT_EQ(barsWeek->high(0), 11);
	EXPECT_EQ(barsWeek->low(0), 1);

	EXPECT_EQ(barsWeek->time(1), sys_days{2018_y/2/12});
	EXPECT_EQ(barsWeek->volume(1), 35);
	EXPECT_EQ(barsWeek->open(1), 11);
	EXPECT_EQ(barsWeek->close(1), 19);
	EXPECT_EQ(barsWeek->high(1), 22);
	EXPECT_EQ(barsWeek->low(1), 10);
}

TEST(TestChart, Macd) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180208,100000,1,10000,1,1317.7,1\n"
		"VTBR,D,20180209,100000,1,10000,1,1315.3,1\n"
		"VTBR,D,20180212,100000,1,10000,1,1317.1,1\n"
		"VTBR,D,20180213,100000,1,10000,1,1316.4,1\n"
		"VTBR,D,20180214,100000,1,10000,1,1319.7,1\n"
		;
	stringstream ss(quotes);

	auto bars = robotrade::parse(ss);
	ASSERT_EQ(bars->num(), 5);

	auto closePrice = data::convertPoints(bars, [&](size_t n) {return bars->close(n);});
	auto testMacd = macd(closePrice, 4,5,3);
	ASSERT_EQ(testMacd->macd->num(), 5);
	ASSERT_EQ(testMacd->signal->num(), 5);
	ASSERT_EQ(testMacd->histogram->num(), 5);

	auto ema4 = ema(closePrice, 4);
	auto ema5 = ema(closePrice, 5);
	EXPECT_EQ(testMacd->macd->close(4), ema4->close(4) - ema5->close(4));

	auto macd = data::convertPoints(ema4, [&](size_t n) {return ema4->close(n) - ema5->close(n);});
	auto ema3 = ema(macd, 3);

	EXPECT_EQ(testMacd->signal->close(4), ema3->close(4));
	EXPECT_EQ(testMacd->histogram->close(4), testMacd->macd->close(4) - testMacd->signal->close(4));
}
