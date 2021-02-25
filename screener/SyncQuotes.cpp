#include <fstream>
#include <iterator>
#include <date/date.h>
#include <util/log.h>
#include <boost/filesystem.hpp>

#include <util/log.h>

#include "SyncQuotes.h"
#include "TiingoApi.h"

using namespace std;

namespace tiingo {

void syncQuotes(istream &tickers, const string &cacheDir, const vector<string> &authTokens, Date from, Date to) {
	int total = 0;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt)
		++total;
	tickers.clear();
	tickers.seekg(0);
	int i = 0;
	int nErrors = 0;
	int nFailed = 0;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		LOG(++i << '/' << total << '\t' << *tickerIt);

		const auto fileName = cacheDir + "/" + *tickerIt;
		if (boost::filesystem::exists(fileName))
			continue;
		ofstream quotesStream(fileName);
		bool failed = true;
		for (const auto &authToken: authTokens)
			try {
				LOG('\t' << authToken);
				TiingoApi tiingoApi(authToken);
				quotesStream << tiingoApi.getData(*tickerIt, from, to);
				failed = false;
				break;
			} catch (const exception &x) {
				++nErrors;
				LOG("Ticker " << *tickerIt << " error " << nErrors << " " << x.what());
			}
		if (failed)
			++nFailed;

	}
	LOG("Sync " << i << " done, errors: " << nErrors << " failed: " << nFailed);
}

}
