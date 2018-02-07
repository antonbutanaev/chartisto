#include <sstream>
#include <gtest/gtest.h>
#include <robotrade/quotesparser.h>

using namespace std;

TEST(TestRobotrade, Parse) {

	string quotes =
		"<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>\n"
		"VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000\n"
		"VTBR,D,20130108,010203,0.0551000,0.0554100,0.0548310,0.0549300,23315540000\n";

	stringstream ss(quotes);
	robotrade::parse(ss);
	EXPECT_EQ(1,1);
}

