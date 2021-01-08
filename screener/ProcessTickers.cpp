#include <iterator>
#include "ProcessTickers.h"

using namespace std;

namespace tiingo {

void processTickers(std::istream &tickers, const std::string &cacheDir) {

	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		cout << *tickerIt << endl;
	}

	cout << cacheDir << endl;
}

}
