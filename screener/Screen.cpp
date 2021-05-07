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
	using RelChange = array<Rate, NumChanges>;
	struct ScreenData {
		string ticker;
		RelStrength relStrength;
		Rate acceleration;
		Rate speed;
		Rate combined;
		Rate acceleration1;
		RelChange relChange;
		int stableGrowth;
	};

	vector<ScreenData> screenDatas;
	screenDatas.reserve(quotess.size());

	array<float, NumPeriods> weights = {1};
	for (auto pN = 1; pN < NumPeriods; ++pN)
		weights[pN] = weights[pN - 1] * WeightPowerRate;

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

		const auto ATR = calcATR(WeightATRRate, endDate - days{RelativeChangePeriod}, endDate, quotes);
		for (auto dN = 0; dN != NumChanges; ++dN) {
			const auto change = calcChange(endDate, dN, quotes);
			screenData.relChange[dN] = ATR? change / ATR : 0;
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
	Rate maxAcceleration1 = 0;
	Rate minAcceleration1 = 0;
	bool first = true;

	for (auto &screenData: screenDatas) {
		screenData.stableGrowth = 0;
		for (auto pN = 0; pN < StableGrowthPeriod && pN < NumPeriods - 1; ++pN) {
			const auto weight = 1 << (StableGrowthPeriod - pN - 1);
			screenData.stableGrowth +=
				screenData.relStrength[pN] > screenData.relStrength[pN + 1] ? weight :
				screenData.relStrength[pN] < screenData.relStrength[pN + 1] ? -weight : 0;
		}

		screenData.acceleration = 0;
		for (auto pN = 0; pN != NumPeriods - 1; ++pN)
			screenData.acceleration +=
				(screenData.relStrength[pN] - screenData.relStrength[pN + 1]) * weights[NumPeriods - pN - 2];

		screenData.acceleration1 = screenData.relStrength[0] - screenData.relStrength[1];

		screenData.speed = 0;
		for (auto pN = 0; pN != NumPeriods; ++pN)
			screenData.speed += screenData.relStrength[pN] * weights[NumPeriods - pN - 1];

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

	for (auto &screenData: screenDatas) {
		if (accelerationSpan)
			screenData.acceleration /= accelerationSpan;
		if (acceleration1Span)
			screenData.acceleration1 /= acceleration1Span;
		if (speedSpan) {
			screenData.speed -= minSpeed;
			screenData.speed /= speedSpan;
		}
		screenData.combined =
			screenParams.accelerationRate * screenData.acceleration +
			(1 - screenParams.accelerationRate) * screenData.speed;
	}

	sort(screenDatas.begin(), screenDatas.end(), [](const auto &a, const auto &b){
		return a.relChange[0] > b.relChange[0];
	});
	const auto tab = '\t';
	for (const auto &screenData: screenDatas) {
		cout
			<< screenData.ticker << tab;
		float totalRelChange = 0;
		for (const auto change: screenData.relChange) {
			cout << change << tab;
			totalRelChange += change;
		}
		cout
			<< totalRelChange << tab
			<< screenData.acceleration1 << tab
			<< screenData.acceleration << tab
			<< screenData.speed << tab
			<< screenData.combined << tab
			<< screenData.stableGrowth << tab;
		for (const auto &rs: screenData.relStrength)
			cout << rs << tab;
		cout << endl;
	}
}

void screen(std::istream &tickers, const string &quotesDir, const ScreenParams &screenParams) {
	screen(parseQuotess(tickers, quotesDir), screenParams);
}

}
