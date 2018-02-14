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

	const auto barsDaily = robotrade::parse(ss);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[](size_t, size_t) {return Action::Wait;}
	);

	const auto result = tripleScreen.run();
	EXPECT_TRUE(result.trades.empty());
}
