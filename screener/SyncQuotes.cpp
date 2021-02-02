#include <fstream>
#include <iterator>
#include <date/date.h>
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
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		ofstream quotesStream(cacheDir + "/" + *tickerIt);
		for (const auto &authToken: authTokens)
			try {
				cout << ++i << '/' << total << '\t' << *tickerIt << '\t' << authToken << endl;
				TiingoApi tiingoApi(authToken);
				quotesStream << tiingoApi.getData(*tickerIt, from, to);
				break;
			} catch (const exception &x) {
				LOG("Ticker " << *tickerIt << " error: " << x.what() << " skip");
			}
	}
}

}
