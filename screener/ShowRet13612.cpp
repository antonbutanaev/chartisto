#include "ShowRet13612.h"
#include "ParseQuotess.h"
#include "Util.h"

using namespace std;

namespace screener {

void showRet13612(const Quotess &quotess) {
	// FIXME eliminate copy paste
	optional<Date> endDate;
	for(const auto &[ticker, quotes]: quotess) {
		const auto tickerEndDate = quotes.back().date;
		if (!endDate)
			endDate = tickerEndDate;
		else if (quotes.back().date != tickerEndDate)
			ERROR(runtime_error, "End date mismatch " << *endDate << tickerEndDate);
	}

	LOG("End date " << *endDate);

	struct Line {
		string ticker;
		Rate ret13612;
	};
	vector<Line> lines;

	for(const auto &[ticker, quotes]: quotess)
		lines.push_back({ticker, calcRet13612W(*endDate, quotes)});

	sort(lines.begin(), lines.end(), [](const auto &a, const auto &b){
		return a.ret13612 > b.ret13612;
	});

	for(const auto &line: lines) {
		cout << line.ticker << '\t' << line.ret13612 << endl;
	}
}

void showRet13612(istream &tickers, const string &quotesDir) {
	showRet13612(parseQuotess(tickers, quotesDir));
}

}
