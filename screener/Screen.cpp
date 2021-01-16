#include <fstream>
#include <sstream>
#include <iterator>
#include <optional>

#include <json/json.h>

#include "Screen.h"

#include <util/log.h>
#include "Util.h"

using namespace std;
using namespace date;

namespace screener {

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
			stringstream dateStream(quoteJson[date].asCString());
			dateStream >> yi >> dash >> mi >> dash >> di;
			if (!dateStream)
				throw runtime_error("could not parse date for " + *tickerIt);
			quotes.push_back({
				year{yi}/mi/di,
				quoteJson[adjOpen].asFloat(),
				quoteJson[adjClose].asFloat(),
				quoteJson[adjHigh].asFloat(),
				quoteJson[adjLow].asFloat(),
				quoteJson[adjVolume].asFloat(),
			});
			LOGn(*tickerIt << ' ' << quotes.back());
		}
	}
	return quotess;
}

void analyzeQuotess(const Quotess &quotess) {
	optional<Date> endDate;
	for(const auto &it: quotess) {
		const auto tickerEndDate = it.second.back().date;
		if (!endDate)
			endDate = tickerEndDate;
		else if (it.second.back().date != tickerEndDate)
			ERROR(runtime_error, "End date mismatch " << *endDate << tickerEndDate);
	}

	LOG("End date " << *endDate);

	struct Ret13612W {
		Ticker ticker;
		float ret13612W;
	};

	vector<Ret13612W> ret13612Ws;
	ret13612Ws.reserve(quotess.size());
	for(const auto &it: quotess) {
		ret13612Ws.push_back({it.first, calcRet13612W(*endDate, it.second)});
	}

	sort(ret13612Ws.begin(), ret13612Ws.end(), [](const auto &a, const auto &b) {return a.ret13612W > b.ret13612W;});

	for (const auto &it: ret13612Ws) {
		LOG(it.ticker << "\t" << it.ret13612W);
	}
}

void screen(std::istream &tickers, const string &quotesDir) {
	analyzeQuotess(parseQuotess(tickers, quotesDir));
}

}
