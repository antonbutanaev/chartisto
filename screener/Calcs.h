#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <chrono>
#include <date/date.h>
#include <util/log.h>

namespace screener {

using Date = date::year_month_day;
using Price = float;
using Volume = float;

constexpr auto epsilon = 1e-6;

struct Quote {
	Date date;
	Price open,close,high,low;
	Volume volume;
};

inline std::ostream &operator<<(std::ostream &o, const Quote &quote) {
	return o
		<< quote.date << ' '
		<< quote.open << ' '
		<< quote.close << ' '
		<< quote.high << ' '
		<< quote.low << ' '
		<< quote.volume;
}

using Ticker = std::string;
using Quotes = std::vector<Quote>;
using QuoteIt = Quotes::const_iterator;
using Quotess = std::map<Ticker, Quotes>;

enum class FindQuoteMode {Exact, GetLastIfNotFound};
QuoteIt findQuote(const Quotes &quotes, Date date, FindQuoteMode mode = FindQuoteMode::Exact);
float calcRet13612W(QuoteIt q0, QuoteIt q1, QuoteIt q3, QuoteIt q6, QuoteIt q12);
float calcMaxDD(QuoteIt qB,  QuoteIt qE);
float calcMaxDD(Date from, Date to, const Quotes &quotes);
float calcRet13612W(Date date, const Quotes &quotes);
float calcRet13612WAdjMaxDD(Date date, const Quotes &quotes);
float calcVol(Date from, Date to, const Quotes &quotes);
float calcRelStrength(Date d, const Quotes &quotes);
float calcChange(Date d, int offset, const Quotes &quotes);
float calcRelativeVolume(Date b, Date e, const Quotes &quotes);
float calcATR(float weightATRRate, Date b, Date e, const Quotes &quotes);

}
