#pragma once

#include "Calcs.h"

namespace screener {

using Date = year_month_day;
using Price = float;
using Volume = float;
using Rate = float;

Quotess parseQuotess(std::istream &tickers, const string &quotesDir);

}
