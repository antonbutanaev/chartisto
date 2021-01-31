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
		ifstream quotesJsonFile(quotesDir + '/' + *tickerIt);
		Json::Value quotesJson;
		try {
			quotesJsonFile >> quotesJson;

			if (!quotesJson.isArray())
				throw runtime_error("expect array json for " + *tickerIt);
		} catch (const exception &x) {
			LOG("Ticker " << *tickerIt << " error: " << x.what() << " skip");
			continue;
		}
		auto &quotes = quotess[*tickerIt];
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
	LOG("Total" + quotess.size());
	return quotess;
}

void analyzeQuotess(const Quotess &quotess) {
	optional<Date> endDate;
	for(const auto &[ticker, quotes]: quotess) {
		const auto tickerEndDate = quotes.back().date;
		if (!endDate)
			endDate = tickerEndDate;
		else if (quotes.back().date != tickerEndDate)
			ERROR(runtime_error, "End date mismatch " << *endDate << tickerEndDate);
	}

	Date halfYearBack = *endDate - months{6};
	LOG("Half year back " << halfYearBack);
	LOG("End date " << *endDate);

	struct Ret13612W {
		Ticker ticker;
		float ret13612W;
		float maxDD;
	};

	vector<Ret13612W> ret13612Ws;
	ret13612Ws.reserve(quotess.size());
	for(const auto &[ticker, quotes]: quotess) {
		const auto ret = calcRet13612W(*endDate, quotes);
		if (ret < 0)
			continue;
		ret13612Ws.push_back({
			ticker,
			ret,
			calcMaxDD(halfYearBack, *endDate, quotes),
		});
	}

	sort(ret13612Ws.begin(), ret13612Ws.end(), [](const auto &a, const auto &b) {
		return a.ret13612W/-a.maxDD > b.ret13612W/-b.maxDD;
	});

	LOG("Ticker\tRet13612W\tMaxDD\tR");
	for (const auto &it: ret13612Ws) {
		LOG(it.ticker << '\t' << it.ret13612W << '\t' << it.maxDD << '\t' << it.ret13612W/it.maxDD);
	}
}

void screen(std::istream &tickers, const string &quotesDir) {
	analyzeQuotess(parseQuotess(tickers, quotesDir));
}

}
