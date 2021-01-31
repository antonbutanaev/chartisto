#include <fstream>
#include <iterator>
#include <date/date.h>
#include <util/log.h>

#include "SyncQuotes.h"
#include "TiingoApi.h"

using namespace std;

namespace tiingo {

void syncQuotes(std::istream &tickers, const std::string &cacheDir, const std::string &authToken, Date from, Date to) {
	int total = 0;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt)
		++total;
	tickers.clear();
	tickers.seekg(0);
	int i = 0;
	TiingoApi tiingoApi(authToken);
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		cout << ++i << '/' << total << '\t' << *tickerIt << endl;
		ofstream quotesStream(cacheDir + "/" + *tickerIt);
		try {
			quotesStream << tiingoApi.getData(*tickerIt, from, to);
		} catch (const exception &x) {
			LOG("Ticker " << *tickerIt << " error: " << x.what() << " skip");
		}
	}
}

}
