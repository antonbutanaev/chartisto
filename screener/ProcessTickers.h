#ifndef SCREENER_PROCESSTICKERS_H_
#define SCREENER_PROCESSTICKERS_H_

#include <iostream>
#include <string>

namespace tiingo {
void processTickers(std::istream &tickers, const std::string &cacheDir, const std::string &authToken);
}

#endif
