#include "ProcessTickers.h"

using namespace std;

namespace tiingo {

void processTickers(std::istream &tickers, const std::string &cacheDir) {
	for (;;) {
		string ticker;
		tickers >> ticker;
		if (!tickers)
			break;
		cout << ticker << endl;
	}

	cout << cacheDir << endl;
}

}
