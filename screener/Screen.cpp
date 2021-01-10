#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <map>
#include <date/date.h>
#include <json/json.h>
#include "Screen.h"

using namespace std;
using namespace date;

namespace tiingo {

#define TPL(a) const auto a = #a
TPL(adjClose);
TPL(adjHigh);
TPL(adjLow);
TPL(adjOpen);
TPL(adjVolume);
TPL(date);
#undef TPL

using Date = year_month_day;
using Price = float;
using Volume = float;

struct Quote {
	Price open,close,high,low;
	Volume volume;
	Date date;
};

ostream &operator<<(ostream &o, const Quote &quote) {
	return o
		<< quote.open << ' '
		<< quote.close << ' '
		<< quote.high << ' '
		<< quote.low << ' '
		<< quote.volume << ' '
		<< quote.date;
}

using Ticker = string;
using Quotes = vector<Quote>;
using Quotess = map<Ticker, Quotes>;

void screen(std::istream &tickers, const string &quotesDir) {

	Quotess quotess;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		cout << *tickerIt << endl;

		auto &quotes = quotess[*tickerIt];

		ifstream quotesJsonFile(quotesDir + '/' + *tickerIt);
		Json::Value quotesJson;
		quotesJsonFile >> quotesJson;
		if (!quotesJson.isArray())
			throw runtime_error("expect array json for " + *tickerIt);
		for (const auto &quoteJson: quotesJson) {
			stringstream dateStr(quoteJson[date].asCString());
			int yi, mi, di;
			char dash;
			dateStr >> yi >> dash >> mi >> dash >> di;
			quotes.push_back({
				quoteJson[adjOpen].asFloat(),
				quoteJson[adjClose].asFloat(),
				quoteJson[adjHigh].asFloat(),
				quoteJson[adjLow].asFloat(),
				quoteJson[adjVolume].asFloat(),
				year{yi}/mi/di
			});
			cout << *tickerIt << ' ' << quotes.back() << endl;
		}
	}
}

}
