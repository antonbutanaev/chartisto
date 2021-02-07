#include <fstream>
#include <sstream>
#include <iterator>
#include <optional>

#include <json/json.h>

#include "Screen.h"

#include <util/log.h>
#include "Util.h"
#include "DateDaysOps.h"

using namespace std;
using namespace date;

namespace screener {

#define TPL(a) const auto a = #a
TPL(adjClose);
TPL(adjHigh);
TPL(adjLow);
TPL(adjOpen);
TPL(adjVolume);
TPL(date);
#undef TPL

using Date = year_month_day;
using Price = float;
using Volume = float;


Quotess parseQuotess(std::istream &tickers, const string &quotesDir) {
	Quotess quotess;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		cout << *tickerIt << endl;
		ifstream quotesJsonFile(quotesDir + '/' + *tickerIt);
		Json::Value quotesJson;
		try {
			quotesJsonFile >> quotesJson;

			if (!quotesJson.isArray())
				throw runtime_error("expect array json for " + *tickerIt);
		} catch (const exception &x) {
			LOG("Ticker " << *tickerIt << " error: " << x.what() << " skip");
			continue;
		}
		auto &quotes = quotess[*tickerIt];
		for (const auto &quoteJson: quotesJson) {
			int yi, mi, di;
			char dash;
			stringstream dateStream(quoteJson[date].asCString());
			dateStream >> yi >> dash >> mi >> dash >> di;
			if (!dateStream)
				throw runtime_error("could not parse date for " + *tickerIt);
			quotes.push_back({
				year{yi}/mi/di,
				quoteJson[adjOpen].asFloat(),
				quoteJson[adjClose].asFloat(),
				quoteJson[adjHigh].asFloat(),
				quoteJson[adjLow].asFloat(),
				quoteJson[adjVolume].asFloat(),
			});
			LOGn(*tickerIt << ' ' << quotes.back());
		}
	}
	LOG("Total" + quotess.size());
	return quotess;
}

void analyzeQuotess(const Quotess &quotess) {
	optional<Date> endDate;
	for(const auto &[ticker, quotes]: quotess) {
		const auto tickerEndDate = quotes.back().date;
		if (!endDate)
			endDate = tickerEndDate;
		else if (quotes.back().date != tickerEndDate)
			ERROR(runtime_error, "End date mismatch " << *endDate << tickerEndDate);
	}

	Date halfYearBack = *endDate - months{6};
	LOG("Half year back " << halfYearBack);
	LOG("End date " << *endDate);

	using RelStrength = array<float, NumPeriods>;
	struct ScreenData {
		string ticker;
		RelStrength relStrength;
		float acceleration;
		float speed;
		float combined;
	};

	vector<ScreenData> screenDatas;
	screenDatas.reserve(quotess.size());

	array<size_t, NumPeriods> fibo;
	fibo[0] = 1;
	fibo[1] = 1;
	for (size_t n = 2; n < NumPeriods; ++n)
		fibo[n] = fibo[n - 1] + fibo[n - 2];

	for(const auto &[ticker, quotes]: quotess) {
		screenDatas.push_back({});
		auto &screenData = screenDatas.back();
		screenData.ticker = ticker;
		Date e = *endDate;
		Date b = e - years{1};
		for (size_t periodNum = 0; periodNum != NumPeriods; ++periodNum, b -= days{Period}, e -= days{Period})
		{
			screenData.relStrength[periodNum] = calcRelStrength(e, quotes);
		}

	}

	for (size_t periodNum = 0; periodNum != NumPeriods; ++periodNum) {
		vector<int> pos;
		for (size_t i=0; i < screenDatas.size(); ++i)
			pos.push_back(i);

		sort(pos.begin(), pos.end(), [&](int a, int b){
			return screenDatas[a].relStrength[periodNum] < screenDatas[b].relStrength[periodNum];
		});

		float relStrength = 1;
		float step = 99. / screenDatas.size();
		for (size_t i=0; i < screenDatas.size(); ++i, relStrength += step)
			screenDatas[pos[i]].relStrength[periodNum] = relStrength;
	}

	float maxSpeed = 1e-6;
	float maxAcceleration = 1e-6;
	for (auto &screenData: screenDatas) {
		screenData.acceleration = 0;
		for (auto periodNum = NumPeriods - 1, fiboN = 0; periodNum != 0; --periodNum, ++fiboN)
			screenData.acceleration += (screenData.relStrength[periodNum - 1] - screenData.relStrength[periodNum]) * fibo[fiboN];

		screenData.speed = 0;
		for (auto periodNum = NumPeriods, fiboN = 0; periodNum != 0; --periodNum, ++fiboN)
			screenData.speed += screenData.relStrength[periodNum - 1] * fibo[fiboN];

		maxAcceleration = max(maxAcceleration, screenData.acceleration);
		maxSpeed = max(maxSpeed, screenData.speed);
	}

	for (const auto &screenData: screenDatas) {
		cout
			<< screenData.ticker << '\t';
		for (const auto &rs: screenData.relStrength)
			cout << rs << '\t';
		cout
			<< screenData.acceleration << '\t'
			<< screenData.speed << '\t'
			<< screenData.acceleration/maxAcceleration * GoldRatioLo + screenData.speed/maxSpeed * GoldRatioHi
			<< endl;
	}
}

void screen(std::istream &tickers, const string &quotesDir) {
	analyzeQuotess(parseQuotess(tickers, quotesDir));
}

}
