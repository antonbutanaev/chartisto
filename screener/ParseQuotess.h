#pragma once

#include "Calcs.h"

namespace screener {

using Rate = float;

Quotess parseQuotess(std::istream &tickers, const std::string &quotesDir);

}
