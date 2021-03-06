#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesParser.h>
#include <robotrade/tripleScreen.h>
#include <robotrade/trader.h>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <date/date.h>
#include <fmt/format.h>
#include <robotrade/entryAnalyzer.h>
#include <chart/stepFind.h>

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

	EXPECT_EQ(bars->title(0), "VTBR");

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

	size_t numTrades = 0;
	Trader trader({1, 2000, [&](Trader::OnTrade) {++numTrades;}});
	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[](size_t, size_t) {return Action::Wait;},
		trader
	);

	tripleScreen.run();
	EXPECT_EQ(numTrades, 0);
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

	vector<Trader::OnTrade> trades;
	Trader trader({100, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});
	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
		},
		trader
	);

	tripleScreen.run();
	ASSERT_EQ(trades.size(), 1);
	const auto &trade = trades.back();
	EXPECT_EQ(trade.time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trade.num, 200);
	EXPECT_EQ(trade.price, 10);
	EXPECT_EQ(trade.gain, NoPrice);
	EXPECT_EQ(trade.total, -2000);
}

TEST(TestRobotrade, TripleScreenBuyThenStopped) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20180215,100000,2,9,2,2,1\n"
		"VTBR,D,20180216,100000,3,10,3,4,1\n"
		"VTBR,D,20180219,100000,5,11,4,6,1\n"
		"VTBR,D,20180220,100000,6,12,6,7,1\n"
		"VTBR,D,20180221,100000,7,13,6,8,1\n"
		"VTBR,D,20180222,100000,7,14,1,1,1\n"
		;
	stringstream ss(quotes);

	const auto barsParsed = robotrade::parse(ss);
	const auto barsDaily = reduce(*barsParsed, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	vector<Trader::OnTrade> trades;
	Trader trader({100, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return barsWeekly->high(weekly) == 10 && barsDaily->close(daily) == 4? Action::Buy : Action::Wait;
		},
		trader
	);

	tripleScreen.run();

	ASSERT_EQ(trades.size(), 2);
	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trades[0].price, 10);
	EXPECT_EQ(trades[0].num, 200);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, -2000);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/22});
	EXPECT_EQ(trades[1].price, 2);
	EXPECT_EQ(trades[1].num, -200);
	EXPECT_EQ(trades[1].gain, -200*(10.-2));
	EXPECT_EQ(trades[1].total, -200*(10.-2));
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

	vector<Trader::OnTrade> trades;
	Trader trader({100, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return barsWeekly->low(weekly) == 19 && barsDaily->close(daily) == 20? Action::Sell : Action::Wait;
		},
		trader
	);

	tripleScreen.run();
	ASSERT_EQ(trades.size(), 1);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trades[0].num, -300);
	EXPECT_EQ(trades[0].price, 19);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 300*19);
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

	vector<Trader::OnTrade> trades;
	Trader trader({100, 2000, [&](const auto &onTrade) {trades.push_back(onTrade);}});

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			return barsWeekly->low(weekly) == 19 && barsDaily->close(daily) == 20? Action::Sell : Action::Wait;
		},
		trader
	);

	tripleScreen.run();
	ASSERT_EQ(trades.size(), 2);

	EXPECT_EQ(trades[0].time, sys_days{2018_y/feb/19});
	EXPECT_EQ(trades[0].num, -300);
	EXPECT_EQ(trades[0].price, 19);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 300*19);

	EXPECT_EQ(trades[1].time, sys_days{2018_y/feb/21});
	EXPECT_EQ(trades[1].num, 300);
	EXPECT_EQ(trades[1].price, 25);
	EXPECT_EQ(trades[1].gain, -300*(25-19));
	EXPECT_EQ(trades[1].total,-300*(25-19));

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
	EXPECT_NEAR(trades[3].gain, ((10*20 + 11*30)/21. - 50)*16, PriceEpsilon);
	EXPECT_EQ(trades[3].total, 10*20 + 11*30 - 5*40 - 16*50);
}

TEST(TestRobotrade, TraderStopBuy) {
	vector<Trader::OnTrade> trades;
	Trader trader({100, 1000, [&](const auto &onTrade) {trades.push_back(onTrade);}});

	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/19}, 100, 99.1});
	ASSERT_EQ(trades.size(), 1);
	EXPECT_EQ(trades[0].num, 1100);
	EXPECT_EQ(trades[0].price, 100);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, -1100 * 100);

	trader.priceChange(sys_days{2018_y/feb/20}, 101);
	ASSERT_EQ(trades.size(), 1);

	trader.priceChange(sys_days{2018_y/feb/20}, 99);
	ASSERT_EQ(trades.size(), 2);
	EXPECT_EQ(trades[1].num, -1100);
	EXPECT_EQ(trades[1].price, 99.1);
	EXPECT_EQ(trades[1].gain, -(100-99.1)*1100);
	EXPECT_NEAR(trades[1].total, -(100-99.1)*1100, PriceEpsilon);
}

TEST(TestRobotrade, TraderStopSell) {
	vector<Trader::OnTrade> trades;
	Trader trader({10, 1000, [&](const auto &onTrade) {trades.push_back(onTrade);}});

	trader.trade({Trader::Trade::ByStop, sys_days{2018_y/feb/19}, 100, 100.7});
	ASSERT_EQ(trades.size(), 1);
	EXPECT_EQ(trades[0].num, -1420);
	EXPECT_EQ(trades[0].price, 100);
	EXPECT_EQ(trades[0].gain, NoPrice);
	EXPECT_EQ(trades[0].total, 142000);

	trader.priceChange(sys_days{2018_y/feb/20}, 99);
	ASSERT_EQ(trades.size(), 1);

	trader.priceChange(sys_days{2018_y/feb/20}, 100.7);
	ASSERT_EQ(trades.size(), 2);
	EXPECT_EQ(trades[1].num, 1420);
	EXPECT_EQ(trades[1].price, 100.7);
	EXPECT_EQ(trades[1].gain, -(100.7-100)*1420);
	EXPECT_NEAR(trades[1].total, -(100.7-100)*1420, PriceEpsilon);
}

TEST(TestRobotrade, EntryAnalyzer) {
	class ProbabilityProvider : public IProbabilityProvider {
	public:
		ProbabilityProvider(unsigned whatHappened) : whatHappened_(whatHappened) {}
		void seed(unsigned) override {}
		unsigned whatHappened(unsigned) override {return whatHappened_;}
	private:
		unsigned whatHappened_;
	};

	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);

		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 26,22,50, 0
		);
		EXPECT_EQ(result.orderActivated, sys_days{2018_y/feb/7});
		EXPECT_NEAR(result.stopEnterPrice, 26, PriceEpsilon);
		EXPECT_NEAR(result.stopPrice, 22, PriceEpsilon);
		EXPECT_NEAR(result.targetPrice, 50, PriceEpsilon);
		EXPECT_FALSE(result.filled);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_FALSE(result.runAway);

	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 24,4,100, 0
		);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_FALSE(result.runAway);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 30,27,100, 0
		);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_FALSE(result.filled);
		ASSERT_TRUE(result.runAway);
		EXPECT_NEAR(result.runAway->price, 24, PriceEpsilon);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,20,10,13.01,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,1, 0
		);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_FALSE(result.filled);
		ASSERT_TRUE(result.runAway);
		EXPECT_NEAR(result.runAway->price, 13, PriceEpsilon);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,24, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_TRUE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		ASSERT_EQ(result.probablyNoProfit.size(), 1u);
		EXPECT_EQ(result.probablyNoProfit[0], sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11.01,4,10,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_TRUE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		ASSERT_EQ(result.probablyNoProfit.size(), 1u);
		EXPECT_EQ(result.probablyNoProfit[0], sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,24, 0
		);
		EXPECT_FALSE(result.stopped);
		ASSERT_EQ(result.probablyNotStopped.size(), 1u);
		EXPECT_EQ(result.probablyNotStopped[0], sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.profit);
		EXPECT_TRUE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11.01,4.99,10,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		EXPECT_FALSE(result.stopped);
		ASSERT_EQ(result.probablyNotStopped.size(), 1u);
		EXPECT_EQ(result.probablyNotStopped[0], sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.profit);
		EXPECT_TRUE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,26, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_TRUE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11.01,6,10,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_TRUE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,26, 0
		);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		ASSERT_EQ(result.probablyNotStopped.size(), 1u);
		EXPECT_EQ(result.probablyNotStopped[0], sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11.01,6,10,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		EXPECT_FALSE(result.stopped);
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		ASSERT_EQ(result.probablyNotStopped.size(), 1u);
		EXPECT_EQ(result.probablyNotStopped[0], sys_days{2018_y/feb/8});
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,5,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,26, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11.01,6,11.01,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,12,10,11,1\n"
			"X,D,20180209,000000,10,25,5,5,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,6,26, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/9});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,10,8,9,10\n"
			"X,D,20180209,000000,10,11.01,9,10,10\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,5, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/9});
		EXPECT_FALSE(result.profit);
		EXPECT_EQ(result.probablyNoProfit.size(), 0u);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,5,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,4,24, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11,4,10,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,12,5, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,5,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,4,24, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		ASSERT_TRUE(result.probablyNoProfit.empty());
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11,4,10,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,12,5, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		ASSERT_TRUE(result.probablyNoProfit.empty());
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,25,5,25,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,4,24, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,11,4,4,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,12,5, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,12,10,11,1\n"
			"X,D,20180209,000000,10,25,5,5,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,4,24, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/9});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,10,8,10,10\n"
			"X,D,20180209,000000,10,11,4,10,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,12,5, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.stopped);
		ASSERT_TRUE(result.profit);
		EXPECT_FALSE(result.profit->probable);
		EXPECT_EQ(result.profit->time, sys_days{2018_y/feb/9});
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,12,10,11,1\n"
			"X,D,20180209,000000,11,20,11,16,1000\n"
			"X,D,20180210,000000,16,20,8,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 11,9,100, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_TRUE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/10});
		ASSERT_FALSE(result.profit);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,10,10,10,10,10\n"
			"X,D,20180208,000000,10,10,8,10,10\n"
			"X,D,20180209,000000,10,10,6,4,1000\n"
			"X,D,20180210,000000,4,11.01,4,10,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(2);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 9,11,1, 0
		);
		ASSERT_TRUE(result.filled);
		EXPECT_EQ(result.filled->time, sys_days{2018_y/feb/8});
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_TRUE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/10});
		ASSERT_FALSE(result.profit);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,23,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Buy, 9,6,24, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		ASSERT_TRUE(result.probablyNoProfit.empty());
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
	{
		string quotes =
			"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
			"X,D,20180207,000000,1,1,1,1,1\n"
			"X,D,20180208,000000,10,23,5,20,1000\n";
		stringstream ss(quotes);

		const auto bars = parse(ss);
		ProbabilityProvider probabilityProvider(1);
		EntryAnalyzer entryAnalyzer({}, bars, probabilityProvider, cout, 1);
		const auto result = entryAnalyzer.analyze(
			EntryAnalyzer::Direction::Sell, 11,22,4, 0
		);
		ASSERT_TRUE(result.stopped);
		EXPECT_FALSE(result.stopped->probable);
		EXPECT_FALSE(result.lossless);
		EXPECT_EQ(result.stopped->time, sys_days{2018_y/feb/8});
		EXPECT_FALSE(result.profit);
		ASSERT_TRUE(result.probablyNoProfit.empty());
		EXPECT_TRUE(result.filled);
		EXPECT_FALSE(result.runAway);
	}
}

TEST(TestRobotrade, RoundUpDown) {
	EXPECT_NEAR(1.01, roundDown(1.0199, 0.01), PriceEpsilon);
	EXPECT_NEAR(1.01, roundDown(1.0101, 0.01), PriceEpsilon);
	EXPECT_NEAR(1.01, roundDown(1.01, 0.01), PriceEpsilon);
	EXPECT_NEAR(1.02, roundUp(1.0199, 0.01), PriceEpsilon);
	EXPECT_NEAR(1.02, roundUp(1.0101, 0.01), PriceEpsilon);
	EXPECT_NEAR(1.02, roundUp(1.02, 0.01), PriceEpsilon);
}
