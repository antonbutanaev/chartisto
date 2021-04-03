#include <numeric>

#include "Calcs.h"

using namespace date;
using namespace std;
using namespace std::chrono;


namespace screener {

QuoteIt findQuote(const Quotes &quotes, Date date, FindQuoteMode mode) {
	auto it = lower_bound(quotes.begin(), quotes.end(), Quote{date}, [](const Quote &a, const Quote &b) {
		return a.date < b.date;
	});
	if (it == quotes.end()) {
		if (mode == FindQuoteMode::Exact)
			ERROR(runtime_error, "Quote for " << date << " not found")
		else
			return --it;
	}
	return it;
}

float calcRet13612W(QuoteIt q0, QuoteIt q1, QuoteIt q3, QuoteIt q6, QuoteIt q12) {
	const auto r1 = q0->close / q1->close - 1;
	const auto r3 = q0->close / q3->close - 1;
	const auto r6 = q0->close / q6->close - 1;
	const auto r12 = q0->close / q12->close - 1;

	return (12 * r1 + 4 * r3 + 2 * r6 + 1 * r12) / 19;
}

float calcMaxDD(QuoteIt qB,  QuoteIt qE) {
	float maxDD = qB->low / qB->high - 1;
	float lastHigh = qB->high;
	for (auto q = qB + 1; q < qE; ++q) {
		lastHigh = max(lastHigh, q->high);
		maxDD = min(maxDD, q->low / lastHigh - 1);
	}
	return maxDD == 0 ? -epsilon : maxDD;
}

float calcMaxDD(Date from, Date to, const Quotes &quotes) {
	return calcMaxDD(findQuote(quotes, from), findQuote(quotes, to));
}

float calcRet13612W(Date date, const Quotes &quotes) {
	return calcRet13612W(
		findQuote(quotes, date),
		findQuote(quotes, date - months{1}),
		findQuote(quotes, date - months{3}),
		findQuote(quotes, date - months{6}),
		findQuote(quotes, date - months{12})
	);
}

float calcRet13612WAdjMaxDD(Date date, const Quotes &quotes) {
	const auto q0 = findQuote(quotes, date);
	const auto q1 = findQuote(quotes, date - months{1});
	const auto q3 = findQuote(quotes, date - months{3});
	const auto q6 = findQuote(quotes, date - months{6});
	const auto q12 = findQuote(quotes, date - months{12});
	const auto ret13612W = calcRet13612W(q0, q1, q3, q6, q12);
	const auto dd1 = calcMaxDD(q1, q0);
	const auto dd3 = calcMaxDD(q3, q0);
	const auto dd6 = calcMaxDD(q6, q0);
	const auto dd12 = calcMaxDD(q12, q0);
	const auto maxDDAvg = (12 * dd1 + 4 * dd3 + 2 * dd6 + 1 * dd12) / 19;
	return ret13612W / -maxDDAvg;
}

float calcVol(Date from, Date to, const Quotes &quotes) {
	const auto qB = findQuote(quotes, from);
	const auto qE = findQuote(quotes, to);
	vector<float> changes;
	for (auto qPrev = qB, q = qB + 1; q < qE; ++q, ++qPrev)
		changes.push_back(q->close / qPrev->close - 1);
	if (changes.empty())
		return epsilon;
	const auto meanChange = accumulate(changes.begin(), changes.end(), 0.) / changes.size();
	const auto variance = accumulate(changes.begin(), changes.end(), 0., [&](auto prev, auto change){
		const auto deviation = change - meanChange;
		return prev + deviation * deviation;

	}) / changes.size();
	return sqrt(variance);

}

float calcRelStrength(Date d, const Quotes &quotes) {
	return calcRet13612WAdjMaxDD(d, quotes);
}

}
