#include <fstream>
#include <iterator>
#include <date/date.h>
#include "ProcessTickers.h"
#include "TiingoApi.h"

using namespace std;

namespace tiingo {

void processTickers(std::istream &tickers, const std::string &cacheDir, const std::string &authToken, Date from, Date to) {
	TiingoApi tiingoApi(authToken);
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		cout << *tickerIt << endl;
		ofstream quotesStream(cacheDir + "/" + *tickerIt);
		quotesStream << tiingoApi.getData(*tickerIt, from, to);
	}
}

}
