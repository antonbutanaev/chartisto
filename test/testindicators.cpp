#include <gtest/gtest.h>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <robotrade/quotesparser.h>
#include <date/date.h>

using namespace chart;
using namespace std;
using namespace date;

TEST(TestIndicators, EMA) {
	class Points: public data::Points {
		std::string title() const override {return "TestEma";}
		Time time(size_t n) const override {return sys_days{2017_y/2/(n+1)};}
		Price close(size_t n) const override {return n+1;}
		size_t num() const override {return 3;}
	};

	const auto emaPoints = indicators::ema(make_shared<Points>(), 3);
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

TEST(TestIndicators, TestDayReduce) {
	EXPECT_EQ(dayReduce(sys_days{2018_y/feb/14} + 20min), sys_days{2018_y/feb/14});
	EXPECT_EQ(dayReduce(sys_days{2018_y/feb/14} + 0min), sys_days{2018_y/feb/14});
	EXPECT_EQ(dayReduce(sys_days{2018_y/feb/14} - 1min), sys_days{2018_y/feb/13});
}

TEST(TestIndicators, TestWeekReduce) {
	EXPECT_EQ(weekReduce(sys_days{2018_y/feb/11} + 23h + 59min), sys_days{2018_y/feb/5});
	EXPECT_EQ(weekReduce(sys_days{2018_y/feb/11} + 23h + 60min), sys_days{2018_y/feb/12});
	EXPECT_EQ(weekReduce(sys_days{2018_y/feb/12} + 20min), sys_days{2018_y/feb/12});
	EXPECT_EQ(weekReduce(sys_days{2018_y/feb/12} + 0min), sys_days{2018_y/feb/12});
	EXPECT_EQ(weekReduce(sys_days{2018_y/feb/12} - 1min), sys_days{2018_y/feb/5});
}

TEST(TestIndicators, Reduce) {
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

TEST(TestIndicators, Macd) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180208,100000,1,10000,1,1317.7,1\n"
		"VTBR,D,20180209,100000,1,10000,1,1315.3,1\n"
		"VTBR,D,20180212,100000,1,10000,1,1317.1,1\n"
		"VTBR,D,20180213,100000,1,10000,1,1316.4,1\n"
		"VTBR,D,20180214,100000,1,10000,1,1319.7,1\n"
		;
	stringstream ss(quotes);

	const auto bars = robotrade::parse(ss);
	ASSERT_EQ(bars->num(), 5);

	const auto closePrice = data::createPoints(bars, [&](size_t n) {return bars->close(n);});
	const auto testMacd = indicators::macd(closePrice, 4,5,3);
	ASSERT_EQ(testMacd->macd->num(), 5);
	ASSERT_EQ(testMacd->signal->num(), 5);
	ASSERT_EQ(testMacd->histogram->num(), 5);

	const auto ema4 = indicators::ema(closePrice, 4);
	const auto ema5 = indicators::ema(closePrice, 5);
	EXPECT_EQ(testMacd->macd->close(4), ema4->close(4) - ema5->close(4));

	const auto macd = data::createPoints(ema4, [&](size_t n) {return ema4->close(n) - ema5->close(n);});
	const auto ema3 = indicators::ema(macd, 3);

	EXPECT_EQ(testMacd->signal->close(4), ema3->close(4));
	EXPECT_EQ(testMacd->histogram->close(4), testMacd->macd->close(4) - testMacd->signal->close(4));
}

TEST(TestIndicators, ForceIndex) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180208,100000,1,10,1,2,1\n"
		"VTBR,D,20180209,100000,1,10,1,3,2\n"
		"VTBR,D,20180212,100000,1,10,1,5,3\n"
		;
	stringstream ss(quotes);

	const auto bars = robotrade::parse(ss);
	ASSERT_EQ(bars->num(), 3);

	const auto forceIndex1 = indicators::forceIndex(bars, 1);
	ASSERT_EQ(forceIndex1->num(), 3);
	ASSERT_EQ(forceIndex1->close(0), NoPrice);
	ASSERT_EQ(forceIndex1->close(1), 2);
	ASSERT_EQ(forceIndex1->close(2), 6);

	const auto forceIndex2 = indicators::forceIndex(bars, 2);
	const auto k = 2. / (1+2);
	ASSERT_EQ(forceIndex2->num(), 3);
	ASSERT_EQ(forceIndex2->close(0), NoPrice);
	ASSERT_EQ(forceIndex2->close(1), 2);
	ASSERT_EQ(forceIndex2->close(2), 6*k + 2*(1-k));
}
