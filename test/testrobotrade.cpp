#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesparser.h>
#include <robotrade/triplescreen.h>
#include <robotrade/trader.h>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <date/date.h>
#include <fmt/format.h>

using namespace std;
using namespace date;
using namespace chart;
using namespace robotrade;
using namespace fmt::literals;

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

TEST(TestRobotrade, TripleScreenBuyNoStop) {
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
			return barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
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
	EXPECT_EQ(trade.stoppedTime, NoTime);
	EXPECT_EQ(trade.maxProfitToStop, 3);
}

TEST(TestRobotrade, TripleScreenBuyThenStopped) {
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
			return barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
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

TEST(TestRobotrade, TripleScreenSellNoStop) {
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
			return barsWeekly->low(weekly) == 19 && barsDaily->close(daily) == 20? Action::Sell : Action::Wait;
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
	EXPECT_EQ(trade.stoppedTime, NoTime);
	EXPECT_EQ(trade.maxProfitToStop, 3);
}

TEST(TestRobotrade, TripleScreenSellThenStopped) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180215,100000,20,25,20,21,1\n"
		"VTBR,D,20180216,100000,19,24,19,20,1\n"
		"VTBR,D,20180219,100000,18,23,18,19,1\n"
		"VTBR,D,20180220,100000,17,22,17,18,1\n"
		"VTBR,D,20180221,100000,16,25,10,17,1\n"
		;
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return barsWeekly->low(weekly) == 19 && barsDaily->close(daily) == 20? Action::Sell : Action::Wait;
		}
	);

	const auto result = tripleScreen.run();
	ASSERT_EQ(result.trades.size(), 1);
	const auto &trade = result.trades.back();
	EXPECT_EQ(trade.time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trade.stoppedTime, sys_days{2018_y/feb/21});
	EXPECT_EQ(trade.number, -1);
	EXPECT_EQ(trade.enterPrice, 19);
	EXPECT_EQ(trade.stopPrice, 25);
	EXPECT_EQ(trade.barsToStop, 2);
	EXPECT_EQ(trade.maxProfitToStop, 9);
}

TEST(TestRobotrade, TraderBuyByStop) {
	vector<Trader::OnTrade> trades;
	Trader trader({1, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/17}, 10, 9});
	ASSERT_EQ(trades.size(), 1);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[0].num, 2000);
	EXPECT_EQ(trades[0].price, 10);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, -20000);
}

TEST(TestRobotrade, TraderSellByStopLot100) {
	vector<Trader::OnTrade> trades;
	Trader trader({100, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/18}, 100, 103});
	ASSERT_EQ(trades.size(), 1);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/18});
	EXPECT_EQ(trades[0].num, -600);
	EXPECT_EQ(trades[0].price, 100);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 60000);
}

TEST(TestRobotrade, TraderBuyCloseLot10) {
	vector<Trader::OnTrade> trades;
	Trader trader({10, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/17}, 10, 7});
	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/17}, 20, 17});
	trader.trade({Trader::Trade::Close, sys_days{2018_y/feb/17}, 40, 41});
	ASSERT_EQ(trades.size(), 3);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[0].num, 660);
	EXPECT_EQ(trades[0].price, 10);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, -6600);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[1].num, 660);
	EXPECT_EQ(trades[1].price, 20);
	EXPECT_EQ(trades[1].gain, NoPrice);
	EXPECT_EQ(trades[1].total, -660*10 - 660*20);

	EXPECT_EQ(trades[2].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[2].num, -660 * 2);
	EXPECT_EQ(trades[2].price, 40);
	EXPECT_EQ(trades[2].gain, 33000);
	EXPECT_EQ(trades[2].total, 33000);
}

TEST(TestRobotrade, TraderBuySellMore) {
	vector<Trader::OnTrade> trades;
	Trader trader({1, 1000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({10, sys_days{2018_y/feb/17}, 100, NoPrice});
	trader.trade({-15, sys_days{2018_y/feb/17}, 90, NoPrice});

	ASSERT_EQ(trades.size(), 2);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[0].num, 10);
	EXPECT_EQ(trades[0].price, 100);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, -1000);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[1].num, -15);
	EXPECT_EQ(trades[1].price, 90);
	EXPECT_EQ(trades[1].gain, -100);
	EXPECT_EQ(trades[1].total, 350);
}

TEST(TestRobotrade, TraderSellBuyMore) {
	vector<Trader::OnTrade> trades;
	Trader trader({1, 1000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({-10, sys_days{2018_y/feb/17}, 20, NoPrice});
	trader.trade({11, sys_days{2018_y/feb/17}, 18, NoPrice});
	trader.trade({Trader::Trade::Close, sys_days{2018_y/feb/17}, 19, NoPrice});

	ASSERT_EQ(trades.size(), 3);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[0].num, -10);
	EXPECT_EQ(trades[0].price, 20);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 200);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[1].num, 11);
	EXPECT_EQ(trades[1].price, 18);
	EXPECT_EQ(trades[1].gain, 20);
	EXPECT_EQ(trades[1].total, 2);

	EXPECT_EQ(trades[2].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[2].num, -1);
	EXPECT_EQ(trades[2].price, 19);
	EXPECT_EQ(trades[2].gain, 1);
	EXPECT_EQ(trades[2].total, 21);
}

TEST(TestRobotrade, TraderAvgCost) {
	vector<Trader::OnTrade> trades;
	Trader trader({1, 1000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	trader.trade({-10, sys_days{2018_y/feb/17}, 20, NoPrice});
	trader.trade({-11, sys_days{2018_y/feb/17}, 30, NoPrice});
	trader.trade({5, sys_days{2018_y/feb/17}, 40, NoPrice});
	trader.trade({Trader::Trade::Close, sys_days{2018_y/feb/17}, 50, NoPrice});

	ASSERT_EQ(trades.size(), 4);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[0].num, -10);
	EXPECT_EQ(trades[0].price, 20);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 200);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[1].num, -11);
	EXPECT_EQ(trades[1].price, 30);
	EXPECT_EQ(trades[1].gain, NoPrice);
	EXPECT_EQ(trades[1].total, 10*20 + 11*30);

	EXPECT_EQ(trades[2].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[2].num, 5);
	EXPECT_EQ(trades[2].price, 40);
	EXPECT_EQ(trades[2].gain, ((10*20 + 11*30)/21. - 40)*5);
	EXPECT_EQ(trades[2].total, 10*20 + 11*30 - 5*40);

	EXPECT_EQ(trades[3].time, sys_days{2018_y/feb/17});
	EXPECT_EQ(trades[3].num, 16);
	EXPECT_EQ(trades[3].price, 50);
	EXPECT_NEAR(trades[3].gain, ((10*20 + 11*30)/21. - 50)*16, 1e-6);
	EXPECT_EQ(trades[3].total, 10*20 + 11*30 - 5*40 - 16*50);
}
