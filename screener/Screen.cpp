#include <util/log.h>

#include "Screen.h"

#include "Calcs.h"
#include "DateDaysOps.h"
#include "ParseQuotess.h"

using namespace std;
using namespace date;

namespace screener {

float flatDiff(float v) {
	return fabs(v) < epsilon? 0. : v > 0 ? 1. : -1.;
}

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
		Rate accelerationFlatDiff;
	};

	vector<ScreenData> screenDatas;
	screenDatas.reserve(quotess.size());

	array<int, NumPeriods> fibo = {1, 1};
	for (auto i = 2; i < NumPeriods; ++i)
		fibo[i] = fibo[i - 1] + fibo[i - 2];

	for(const auto &[ticker, quotes]: quotess) {
		screenDatas.push_back({ticker});
		for (auto [pN, e] = make_tuple(0, endDate); pN != NumPeriods; ++pN, e -= days{Period})
			try {
				screenDatas.back().relStrength[pN] = calcRelStrength(e, quotes);
			} catch (const exception &x) {
				LOG("Error for " << ticker << ": " << x.what());
				throw;
			}
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
	Rate maxAccelerationFlatDiff = 0;
	Rate minAccelerationFlatDiff = 0;
	bool first = true;

	for (auto &screenData: screenDatas) {
		screenData.acceleration = 0;
		for (auto pN = 0; pN != NumPeriods - 1; ++pN)
			screenData.acceleration +=
				(screenData.relStrength[pN] - screenData.relStrength[pN + 1]) * fibo[NumPeriods - pN - 2];

		screenData.accelerationFlatDiff = 0;
		for (auto pN = 0; pN != NumPeriods - 1; ++pN)
			screenData.accelerationFlatDiff +=
				flatDiff(screenData.relStrength[pN] - screenData.relStrength[pN + 1]) * fibo[NumPeriods - pN - 2];

		screenData.speed = 0;
		for (auto pN = 0; pN != NumPeriods; ++pN)
			screenData.speed += screenData.relStrength[pN] * fibo[NumPeriods - pN - 1];

		if (first) {
			minAcceleration = maxAcceleration = screenData.acceleration;
			minAccelerationFlatDiff = maxAccelerationFlatDiff = screenData.accelerationFlatDiff;
			minSpeed = maxSpeed = screenData.speed;
		} else {
			maxAcceleration = max(maxAcceleration, screenData.acceleration);
			minAcceleration = min(minAcceleration, screenData.acceleration);
			maxAccelerationFlatDiff = max(maxAccelerationFlatDiff, screenData.accelerationFlatDiff);
			minAccelerationFlatDiff = min(minAccelerationFlatDiff, screenData.accelerationFlatDiff);
			maxSpeed = max(maxSpeed, screenData.speed);
			minSpeed = min(minSpeed, screenData.speed);
		}
		first = false;
	}

	const auto speedSpan = maxSpeed - minSpeed;
	const auto accelerationSpan = maxAcceleration - minAcceleration;
	const auto accelerationFlatDiffSpan = maxAccelerationFlatDiff - minAccelerationFlatDiff;

	if (accelerationFlatDiffSpan == 0 || accelerationSpan == 0 || speedSpan == 0)
		ERROR(runtime_error, "Impossible to normalize, span is zero");

	for (auto &screenData: screenDatas) {
		screenData.acceleration /= accelerationSpan;
		screenData.accelerationFlatDiff /= accelerationFlatDiffSpan;
		screenData.speed -= minSpeed;
		screenData.speed /= speedSpan;
		screenData.combined =
			screenParams.accelerationRate * screenData.acceleration +
			(1 - screenParams.accelerationRate) * screenData.speed;
	}

	sort(screenDatas.begin(), screenDatas.end(), [](const auto &a, const auto &b){
		return a.combined > b.combined;
	});
	for (const auto &screenData: screenDatas) {
		cout
			<< screenData.ticker << '\t';
		for (const auto &rs: screenData.relStrength)
			cout << rs << '\t';
		cout
			<< screenData.acceleration << '\t'
			<< screenData.accelerationFlatDiff << '\t'
			<< screenData.speed << '\t'
			<< screenData.combined << endl;
	}
}

void screen(std::istream &tickers, const string &quotesDir, const ScreenParams &screenParams) {
	screen(parseQuotess(tickers, quotesDir), screenParams);
}

}
