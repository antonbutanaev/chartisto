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
	Date date;
	Price open,close,high,low;
	Volume volume;
};

ostream &operator<<(ostream &o, const Quote &quote) {
	return o
		<< quote.date << ' '
		<< quote.open << ' '
		<< quote.close << ' '
		<< quote.high << ' '
		<< quote.low << ' '
		<< quote.volume;
}

using Ticker = string;
using Quotes = vector<Quote>;
using Quotess = map<Ticker, Quotes>;

auto findQuote(const Quotes &quotes, Date date) {
	return lower_bound(quotes.begin(), quotes.end(), Quote{date}, [](const Quote &a, const Quote &b) {
		return a.date < b.date;
	});
}

Quotess parseQuotess(std::istream &tickers, const string &quotesDir) {
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
			int yi, mi, di;
			char dash;
			stringstream(quoteJson[date].asCString()) >> yi >> dash >> mi >> dash >> di;
			quotes.push_back({
				year{yi}/mi/di,
				quoteJson[adjOpen].asFloat(),
				quoteJson[adjClose].asFloat(),
				quoteJson[adjHigh].asFloat(),
				quoteJson[adjLow].asFloat(),
				quoteJson[adjVolume].asFloat(),
			});
			//cout << *tickerIt << ' ' << quotes.back() << endl;
		}
	}
	return quotess;
}

void analyzeQuotess(const Quotess &quotess) {
	for(const auto &it: quotess) {
		const auto it1 = findQuote(it.second, year{2020}/10/1);
		const auto it2 = findQuote(it.second, year{2020}/2/1);
		if (it1 != it.second.end() && it2 != it.second.end())
			cout << it.first << ' ' << *it1 << ' ' << *it2 << endl;
		break;
	}

}

void screen(std::istream &tickers, const string &quotesDir) {
	analyzeQuotess(parseQuotess(tickers, quotesDir));
}

}
