#include <iomanip>
#include "ShowRet13612.h"

#include "Calcs.h"
#include "ParseQuotess.h"

using namespace std;

namespace screener {

auto round4(double v) {
	const auto prec4 = 10000;
	return floor(prec4 * v + .5) / prec4;
}

void showRet13612(const Quotess &quotess) {
	const auto endDate = quotess.begin()->second.back().date;
	LOG("End date " << endDate);

	struct Line {
		string ticker;
		Rate ret13612;
	};
	vector<Line> lines;
	lines.reserve(quotess.size());

	for(const auto &[ticker, quotes]: quotess)
		lines.push_back({ticker, calcRet13612W(endDate, quotes)});

	sort(lines.begin(), lines.end(), [](const auto &a, const auto &b){
		return a.ret13612 > b.ret13612;
	});

	cout << setprecision(4);
	for(const auto &line: lines)
		cout << line.ticker << '\t' << round4(line.ret13612) << endl;
}

void showRet13612(istream &tickers, const string &quotesDir) {
	showRet13612(parseQuotess(tickers, quotesDir));
}

}
