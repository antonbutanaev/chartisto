#include <fstream>
#include <json/json.h>
#include <util/log.h>

#include "Util.h"
#include "ParseQuotess.h"
#include "DateDaysOps.h"

namespace screener {

#define TPL(a) const auto a = #a
TPL(adjClose);
TPL(adjHigh);
TPL(adjLow);
TPL(adjOpen);
TPL(adjVolume);
TPL(date);
#undef TPL

Quotess parseQuotess(std::istream &tickers, const string &quotesDir) {
	Quotess quotess;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		LOGn(*tickerIt);
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
			quotes.push_back({
				stringToDate(quoteJson[date].asString()),
				quoteJson[adjOpen].asFloat(),
				quoteJson[adjClose].asFloat(),
				quoteJson[adjHigh].asFloat(),
				quoteJson[adjLow].asFloat(),
				quoteJson[adjVolume].asFloat(),
			});
			LOGn(*tickerIt << ' ' << quotes.back());
		}
	}
	LOGn("Total" + quotess.size());
	if (quotess.empty())
		ERROR(runtime_error, "Empty quotes");

	optional<Date> endDate;
	for(const auto &[ticker, quotes]: quotess) {
		const auto tickerEndDate = quotes.back().date;
		if (!endDate)
			endDate = tickerEndDate;
		else if (quotes.back().date != tickerEndDate)
			ERROR(runtime_error, "End date mismatch " << *endDate << tickerEndDate);
	}

	return quotess;
}

}
