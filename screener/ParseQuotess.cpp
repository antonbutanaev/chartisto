#include <fstream>
#include <json/json.h>
#include <util/log.h>

#include "Util.h"
#include "ParseQuotess.h"

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
	LOGn("Total" + quotess.size());
	return quotess;
}

}
