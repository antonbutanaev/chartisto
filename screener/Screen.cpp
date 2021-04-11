#include <util/log.h>

#include "Screen.h"

#include "Calcs.h"
#include "DateDaysOps.h"
#include "ParseQuotess.h"

using namespace std;
using namespace date;

namespace screener {

void screen(const Quotess &quotess, const ScreenParams &screenParams) {
	const auto endDate = findQuote(quotess.begin()->second, screenParams.toDate, FindQuoteMode::GetLastIfNotFound)->date;
	LOG("On date: " << screenParams.toDate << " Actual: " << endDate);

	using RelStrength = array<Rate, NumPeriods>;
	struct ScreenData {
		string ticker;
		RelStrength relStrength;
		Rate acceleration;
		Rate speed;
		Rate combined;
		Rate acceleration1;
		Rate change;
		Rate relativeVolume;
		Rate relativeChange;
	};

	vector<ScreenData> screenDatas;
	screenDatas.reserve(quotess.size());

	array<int, NumPeriods> fibo = {1, 1};
	for (auto i = 2; i < NumPeriods; ++i)
		fibo[i] = fibo[i - 1] + fibo[i - 2];

	for(const auto &[ticker, quotes]: quotess) {
		screenDatas.push_back({ticker});
		auto &screenData = screenDatas.back();
		for (auto [pN, e] = make_tuple(0, endDate); pN != NumPeriods; ++pN, e -= days{Period}) {
			try {
				screenData.relStrength[pN] = calcRelStrength(e, quotes);
			} catch (const exception &x) {
				LOG("Error for " << ticker << ": " << x.what());
				throw;
			}
		}

		screenData.change = calcChange(endDate, quotes);
		screenData.relativeVolume = calcRelativeVolume(endDate - RelativeVolumePeriod, endDate, quotes);
		screenData.relativeChange = calcRelativeChange(endDate - RelativeChangePeriod, endDate, quotes);
	}

	for (auto pN = 0; pN != NumPeriods; ++pN) {
		vector<decltype(screenDatas.begin())> screenDataIts;
		screenDataIts.reserve(screenDatas.size());
		auto genIt = screenDatas.begin();
		generate_n(back_inserter(screenDataIts), screenDatas.size(), [&]{return genIt++;});
		sort(screenDataIts.begin(), screenDataIts.end(), [&](auto a, auto b){
			return a->relStrength[pN] < b->relStrength[pN];
		});

		auto relStrength = 1.;
		const auto step = screenDataIts.empty()? 0. : 99. / (screenDataIts.size() - 1);
		for (auto &it: screenDataIts) {
			it->relStrength[pN] = relStrength;
			relStrength += step;
		}
	}

	Rate maxSpeed = 0;
	Rate minSpeed = 0;
	Rate maxAcceleration = 0;
	Rate minAcceleration = 0;
	Rate maxAcceleration1 = 0;
	Rate minAcceleration1 = 0;
	bool first = true;

	for (auto &screenData: screenDatas) {
		screenData.acceleration = 0;
		for (auto pN = 0; pN != NumPeriods - 1; ++pN)
			screenData.acceleration +=
				(screenData.relStrength[pN] - screenData.relStrength[pN + 1]) * fibo[NumPeriods - pN - 2];

		screenData.acceleration1 = screenData.relStrength[0] - screenData.relStrength[1];

		screenData.speed = 0;
		for (auto pN = 0; pN != NumPeriods; ++pN)
			screenData.speed += screenData.relStrength[pN] * fibo[NumPeriods - pN - 1];

		if (first) {
			minAcceleration = maxAcceleration = screenData.acceleration;
			minAcceleration1 = maxAcceleration1 = screenData.acceleration1;
			minSpeed = maxSpeed = screenData.speed;
		} else {
			maxAcceleration = max(maxAcceleration, screenData.acceleration);
			minAcceleration = min(minAcceleration, screenData.acceleration);
			maxAcceleration1 = max(maxAcceleration1, screenData.acceleration1);
			minAcceleration1 = min(minAcceleration1, screenData.acceleration1);
			maxSpeed = max(maxSpeed, screenData.speed);
			minSpeed = min(minSpeed, screenData.speed);
		}
		first = false;
	}

	const auto speedSpan = maxSpeed - minSpeed;
	const auto accelerationSpan = maxAcceleration - minAcceleration;
	const auto acceleration1Span = maxAcceleration1 - minAcceleration1;

	if (acceleration1Span == 0 || accelerationSpan == 0 || speedSpan == 0)
		ERROR(runtime_error, "Impossible to normalize, span is zero");

	for (auto &screenData: screenDatas) {
		screenData.acceleration /= accelerationSpan;
		screenData.acceleration1 /= acceleration1Span;
		screenData.speed -= minSpeed;
		screenData.speed /= speedSpan;
		screenData.combined =
			screenParams.accelerationRate * screenData.acceleration +
			(1 - screenParams.accelerationRate) * screenData.speed;
	}

	sort(screenDatas.begin(), screenDatas.end(), [](const auto &a, const auto &b){
		return a.change > b.change;
	});
	const auto tab = '\t';
	for (const auto &screenData: screenDatas) {
		cout
			<< screenData.ticker << tab;
		cout
			<< screenData.change << tab
			<< screenData.relativeChange << tab
			<< screenData.relativeVolume << tab
			<< screenData.acceleration1 << tab
			<< screenData.acceleration << tab
			<< screenData.speed << tab
			<< screenData.combined << tab;
		for (const auto &rs: screenData.relStrength)
			cout << rs << tab;
		cout << endl;
	}
}

void screen(std::istream &tickers, const string &quotesDir, const ScreenParams &screenParams) {
	screen(parseQuotess(tickers, quotesDir), screenParams);
}

}
