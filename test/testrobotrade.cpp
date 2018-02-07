#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesparser.h>
#include <date.h>

using namespace std;
using namespace date;

TEST(TestRobotrade, Parse) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000\n"
		"VTBR,D,20130109,010203,0.0551000,0.0554100,0.0548310,0.0549300,23315540000\n";
	stringstream ss(quotes);

	int quoteNum = 0;
	robotrade::parse(ss, [&](const auto &bar){
		switch(++quoteNum) {
		case 1:
			EXPECT_EQ(bar.ticker, "VTBR");
			EXPECT_EQ(bar.bar.time(), sys_days{2013_y/1/8});
			EXPECT_EQ(bar.bar.open(), .055);
			EXPECT_EQ(bar.bar.high(), .0554);
			EXPECT_EQ(bar.bar.low(), .05483);
			EXPECT_EQ(bar.bar.close(), .05492);
			EXPECT_EQ(bar.bar.volume(), 23315530000);
			break;
		case 2:
			EXPECT_EQ(bar.ticker, "VTBR");
			EXPECT_EQ(bar.bar.time(), sys_days{2013_y/1/9} + 1h + 2min);
			EXPECT_EQ(bar.bar.open(), .0551);
			EXPECT_EQ(bar.bar.high(), .05541);
			EXPECT_EQ(bar.bar.low(), .054831);
			EXPECT_EQ(bar.bar.close(), .05493);
			EXPECT_EQ(bar.bar.volume(), 23315540000);
			break;
		}
	});
	EXPECT_EQ(quoteNum, 2);
}

