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
using Rate = float;

Quotess parseQuotess(std::istream &tickers, const string &quotesDir) {
	Quotess quotess;
	for (auto tickerIt = istream_iterator<string>(tickers), end = istream_iterator<string>(); tickerIt != end; ++tickerIt) {
		LOG(*tickerIt);
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
	LOGn("Total" + quotess.size());
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

	LOG("End date " << *endDate);

	using RelStrength = array<Rate, NumPeriods>;
	struct ScreenData {
		string ticker;
		RelStrength relStrength;
		Rate acceleration;
		Rate speed;
		Rate combined;
	};

	vector<ScreenData> screenDatas;
	screenDatas.reserve(quotess.size());

	array<int, NumPeriods> fibo = {1,1};
	for (auto i = 2; i < NumPeriods; ++i)
		fibo[i] = fibo[i - 1] + fibo[i - 2];

	for(const auto &[ticker, quotes]: quotess) {
		screenDatas.push_back({});
		auto &screenData = screenDatas.back();
		screenData.ticker = ticker;
		Date e = *endDate;
		for (size_t periodNum = 0; periodNum != NumPeriods; ++periodNum, e -= days{Period})
			screenData.relStrength[periodNum] = calcRelStrength(e, quotes);
	}

	for (size_t pN = 0; pN != NumPeriods; ++pN) {
		vector<decltype(screenDatas.begin())> screenDataIts;
		screenDataIts.reserve(screenDatas.size());
		auto genIt = screenDatas.begin();
		generate_n(back_inserter(screenDataIts), screenDatas.size(), [&]{return genIt++;});
		sort(screenDataIts.begin(), screenDataIts.end(), [&](auto a, auto b){
			return a->relStrength[pN] < b->relStrength[pN];
		});

		auto relStrength = 1.;
		const auto step = 99. / (screenDataIts.size() - 1);
		for (auto &it: screenDataIts) {
			it->relStrength[pN] = relStrength;
			relStrength += step;
		}
	}

	Rate maxSpeed;
	Rate maxAcceleration;
	Rate minAcceleration;
	bool first = true;

	for (auto &screenData: screenDatas) {
		screenData.acceleration = 0;
		for (auto pN = 0; pN != NumPeriods - 1; ++pN)
			screenData.acceleration +=
				(screenData.relStrength[pN] - screenData.relStrength[pN + 1]) * fibo[NumPeriods - pN - 2];

		screenData.speed = 0;
		for (auto pN = 0; pN != NumPeriods; ++pN)
			screenData.speed += screenData.relStrength[pN] * fibo[NumPeriods - pN - 1];

		if (first) {
			maxAcceleration = screenData.acceleration;
			minAcceleration = screenData.acceleration;
			maxSpeed = screenData.speed;
		} else {
			maxAcceleration = max(maxAcceleration, screenData.acceleration);
			minAcceleration = min(minAcceleration, screenData.acceleration);
			maxSpeed = max(maxSpeed, screenData.speed);
		}
		first = false;
	}

	for (auto &screenData: screenDatas) {
		screenData.combined =
			screenData.acceleration / (maxAcceleration - minAcceleration) * .5 +
			screenData.speed / maxSpeed * .5;
	}

	for (const auto &screenData: screenDatas) {
		cout
			<< screenData.ticker << '\t';
		for (const auto &rs: screenData.relStrength)
			cout << rs << '\t';
		cout
			<< screenData.acceleration << '\t'
			<< screenData.speed << '\t'
			<< screenData.combined << endl;
	}
}

void screen(std::istream &tickers, const string &quotesDir) {
	analyzeQuotess(parseQuotess(tickers, quotesDir));
}

}
