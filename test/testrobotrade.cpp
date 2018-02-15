#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesparser.h>
#include <robotrade/triplescreen.h>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <date/date.h>

using namespace std;
using namespace date;
using namespace chart;
using namespace robotrade;

TEST(TestRobotrade, Parse) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000\n"
		"VTBR,D,20130109,010203,0.0551000,0.0554100,0.0548310,0.0549300,23315540000\n";
	stringstream ss(quotes);

	const auto bars = parse(ss);

	ASSERT_EQ(bars->num(), 2);

	EXPECT_EQ(bars->title(), "VTBR");

	EXPECT_EQ(bars->time(0), sys_days{2013_y/1/8});
	EXPECT_EQ(bars->open(0), .055);
	EXPECT_EQ(bars->high(0), .0554);
	EXPECT_EQ(bars->low(0), .05483);
	EXPECT_EQ(bars->close(0), .05492);
	EXPECT_EQ(bars->volume(0), 23315530000);

	EXPECT_EQ(bars->time(1), sys_days{2013_y/1/9} + 1h + 2min);
	EXPECT_EQ(bars->open(1), .0551);
	EXPECT_EQ(bars->high(1), .05541);
	EXPECT_EQ(bars->low(1), .054831);
	EXPECT_EQ(bars->close(1), .05493);
	EXPECT_EQ(bars->volume(1), 23315540000);
}

TEST(TestRobotrade, TripleScreenEmpty) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000\n"
		"VTBR,D,20130109,010203,0.0551000,0.0554100,0.0548310,0.0549300,23315540000\n";
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[](size_t, size_t) {return Action::Wait;}
	);

	const auto result = tripleScreen.run();
	EXPECT_TRUE(result.trades.empty());
}

TEST(TestRobotrade, TripleScreenBuy1) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180215,100000,2,9,2,2,1\n"
		"VTBR,D,20180216,100000,3,10,1,4,1\n"
		"VTBR,D,20180219,100000,5,11,2,6,1\n"
		"VTBR,D,20180220,100000,6,12,6,7,1\n"
		"VTBR,D,20180221,100000,7,13,6,8,1\n"
		;
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return
				barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
		}
	);

	const auto result = tripleScreen.run();
	ASSERT_EQ(result.trades.size(), 1);
	const auto &trade = result.trades.back();
	EXPECT_EQ(trade.time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trade.number, 1);
	EXPECT_EQ(trade.enterPrice, 10);
	EXPECT_EQ(trade.stopPrice, 1);
	EXPECT_EQ(trade.barsToStop, StrategyResult::Trade::NoStop);
	EXPECT_EQ(trade.maxProfitToStop, 3);
}

TEST(TestRobotrade, TripleScreenBuy2) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180215,100000,2,9,2,2,1\n"
		"VTBR,D,20180216,100000,3,10,3,4,1\n"
		"VTBR,D,20180219,100000,5,11,4,6,1\n"
		"VTBR,D,20180220,100000,6,12,6,7,1\n"
		"VTBR,D,20180221,100000,7,13,6,8,1\n"
		"VTBR,D,20180222,100000,7,14,2,1,1\n"
		;
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return
				barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
		}
	);

	const auto result = tripleScreen.run();
	ASSERT_EQ(result.trades.size(), 1);
	const auto &trade = result.trades.back();
	EXPECT_EQ(trade.time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trade.stoppedTime, sys_days{2018_y/feb/22});
	EXPECT_EQ(trade.number, 1);
	EXPECT_EQ(trade.enterPrice, 10);
	EXPECT_EQ(trade.stopPrice, 2);
	EXPECT_EQ(trade.barsToStop, 3);
	EXPECT_EQ(trade.maxProfitToStop, 4);
}

TEST(TestRobotrade, TripleScreenSell1) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180215,100000,20,25,20,21,1\n"
		"VTBR,D,20180216,100000,19,24,19,20,1\n"
		"VTBR,D,20180219,100000,18,23,18,19,1\n"
		"VTBR,D,20180220,100000,17,22,17,18,1\n"
		"VTBR,D,20180221,100000,16,21,16,17,1\n"
		;
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return
				barsWeekly->low(weekly) == 19 && barsDaily->close(daily) == 20? Action::Sell : Action::Wait;
		}
	);

	const auto result = tripleScreen.run();
	ASSERT_EQ(result.trades.size(), 1);
	const auto &trade = result.trades.back();
	EXPECT_EQ(trade.time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trade.number, -1);
	EXPECT_EQ(trade.enterPrice, 19);
	EXPECT_EQ(trade.stopPrice, 25);
	EXPECT_EQ(trade.barsToStop, StrategyResult::Trade::NoStop);
	EXPECT_EQ(trade.maxProfitToStop, 3);
}
