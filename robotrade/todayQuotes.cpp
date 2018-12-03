#include <fstream>
#include <json/value.h>
#include <robotrade/todayQuotes.h>
#include <robotrade/quotesParser.h>
#include <util/stream.h>

using namespace std;
using namespace util;

namespace robotrade {

chart::data::PBars parseQuotesWithToday(
	const std::string &quotes,
	const std::string &todayQuotesDir,
	const Json::Value &titleToTickers
) {
	auto ifs = check<ifstream>(quotes);
	return robotrade::parse(ifs, [&](const string &title){
		return make_unique<ifstream>(check<ifstream>(
			todayQuotesDir + '/' + titleToTickers[title].asString())
		);
	});
}

}
