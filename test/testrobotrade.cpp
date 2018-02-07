#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesparser.h>
#include <date.h>

using namespace std;
using namespace date;

TEST(TestRobotrade, Parse) {
	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\r\n"
		"VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000\r\n"
		"VTBR,D,20130109,010203,0.0551000,0.0554100,0.0548310,0.0549300,23315540000\r\n";

	stringstream ss(quotes);

	int quoteNum = 0;
	robotrade::parse(ss, [&](const robotrade::Bar &bar){
		switch(++quoteNum) {
		case 1:
			EXPECT_EQ(bar.ticker, "VTBR");
			EXPECT_EQ(bar.bar.time(), sys_days{2013_y/1/8});
			break;
		case 2:
			EXPECT_EQ(bar.ticker, "VTBR");
			EXPECT_EQ(bar.bar.time(), sys_days{2013_y/1/9} + 1h + 2min);
			break;
		}
	});
	EXPECT_EQ(quoteNum, 2);
}

