#ifndef SCREENER_PROCESSTICKERS_H_
#define SCREENER_PROCESSTICKERS_H_

#include <date/date.h>
#include <iostream>
#include <string>

namespace tiingo {

using Date = date::year_month_day;
void syncQuotes(std::istream &tickers, const std::string &cacheDir, const std::vector<std::string> &authTokens, Date from, Date to);

}

#endif
