#ifndef SCREENER_UTIL_H_
#define SCREENER_UTIL_H_

#include <vector>
#include <numeric>
#include <map>
#include <iostream>
#include <chrono>

#include <date/date.h>

#include <util/log.h>

namespace screener {

using namespace date;
using namespace std;
using namespace std::chrono;

using Date = year_month_day;
using Price = float;
using Volume = float;

struct Quote {
	Date date;
	Price open,close,high,low;
	Volume volume;
};

std::ostream &operator<<(std::ostream &o, const Quote &quote) {
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
using Quotess = std::map<Ticker, Quotes>;

auto findQuote(const Quotes &quotes, Date date, bool stepBack = true) {
	auto it = lower_bound(quotes.begin(), quotes.end(), Quote{date}, [](const Quote &a, const Quote &b) {
		return a.date < b.date;
	});
	if (it == quotes.end())
		ERROR(runtime_error, "Quote for " << date << " not found");
	if (stepBack && it != quotes.begin())
		--it;
	return it;
}

float calcRet13612W(Date date, const Quotes &quotes) {
	const auto q0 = findQuote(quotes, date, false);
	const auto q1 = findQuote(quotes, date - months{1});
	const auto q3 = findQuote(quotes, date - months{3});
	const auto q6 = findQuote(quotes, date - months{6});
	const auto q12 = findQuote(quotes, date - months{12});

	const auto r1 = q0->close / q1->close - 1;
	const auto r3 = q0->close / q3->close - 1;
	const auto r6 = q0->close / q6->close - 1;
	const auto r12 = q0->close / q12->close - 1;

	return (
		12 * r1 +
		4 * r3 +
		2 * r6 +
		1 * r12
	) / 4;
}

float calcVol(Date from, Date to, const Quotes &quotes) {
	const auto qB = findQuote(quotes, from);
	const auto qE = findQuote(quotes, to);
	vector<float> changes;
	for (auto qPrev = qB, q = qB + 1; q < qE; ++q, ++qPrev)
		changes.push_back(q->close / qPrev->close - 1);
	if (changes.empty())
		return 1e-6;
	const auto meanChange = accumulate(changes.begin(), changes.end(), 0.) / changes.size();
	const auto variance = accumulate(changes.begin(), changes.end(), 0., [&](auto prev, auto change){
		const auto deviation = change - meanChange;
		return prev + deviation * deviation;

	}) / changes.size();
	return sqrt(variance);

}

float calcMaxDD(Date from, Date to, const Quotes &quotes) {
	const auto qB = findQuote(quotes, from);
	const auto qE = findQuote(quotes, to);
	float maxDD = qB->low / qB->high - 1;
	float lastHigh = qB->high;
	for (auto q = qB + 1; q != qE; ++q) {
		lastHigh = max(lastHigh, q->high);
		maxDD = min(maxDD, q->low/lastHigh - 1);
	}
	return maxDD == 0 ? -1e-6 : maxDD;
}

}

#endif
