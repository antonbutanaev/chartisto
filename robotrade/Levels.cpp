#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>
#include <json/json.h>
#include <robotrade/Levels.h>

using namespace std;
using namespace chart;

namespace robotrade {

struct FindLevelsParams {
	float priceRangeK = 0.2;
	size_t minTouches = 3;
	Price precisionK = 0.001;
	Price step = 0.01;
	double touchWeight = 1;
	double crossWeight = -2;
	double sameLevelK = 0.03;
	size_t maxlevels = 10;
	size_t minExtremumAgeBars = 20;
	size_t extremumNumTouches = 100;
};


struct Level {
	size_t numTouches;
	size_t numBodyCrosses;
	Price level;
};

void findLevels(data::PBars bars, size_t from, size_t to, const std::string &config) {
	cout << "Using config " << config << endl;
	ifstream ifs(config);
	if (!ifs)
		throw runtime_error("Could not read config " + config);
	Json::Value configJson;
	ifs >> configJson;

	FindLevelsParams params;
	std::vector<Level> levels;
	if (from >= to)
		return;

	auto minPrice = numeric_limits<Price>::max();
	auto maxPrice = numeric_limits<Price>::lowest();
	size_t minPriceNum = from;
	size_t maxPriceNum = from;
	for (auto barNum = from; barNum < to; ++barNum) {
		if (minPrice > bars->low(barNum)) {
			minPrice = bars->low(barNum);
			minPriceNum = barNum;
		}
		if (maxPrice < bars->high(barNum)) {
			maxPrice = bars->high(barNum);
			maxPriceNum = barNum;
		}
	}

	auto rangeLow = bars->close(to - 1) * (1 - params.priceRangeK);
	auto rangeHigh = bars->close(to - 1) * (1 + params.priceRangeK);
	rangeLow = ceil(rangeLow / params.step) * params.step;
	rangeHigh = floor(rangeHigh / params.step) * params.step;

	rangeLow = std::max(minPrice, rangeLow);
	rangeHigh = std::min(maxPrice, rangeHigh);

	cout << "Price range: " << minPrice << " " << maxPrice << endl;

	if (minPrice == rangeLow && minPriceNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			minPrice
		});

	if (maxPrice == rangeHigh && maxPriceNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			maxPrice
		});

	for (auto price = rangeLow; price <= rangeHigh; price += params.step) {
		Level level{0, 0, price};
		for (size_t barNum = from; barNum < to; ++barNum) {
			const auto upperBound = price * (1 + params.precisionK);
			const auto lowerBound = price * (1 - params.precisionK);

			for (const auto &priceType: data::Bars::PriceTypes) {
				const auto barPrice = bars->get(priceType, barNum);
				if (barPrice >= lowerBound && barPrice <= upperBound) {
					level.numTouches += 1;
					break;
				}
			}

			const auto barOpen = bars->open(barNum);
			const auto barClose = bars->close(barNum);
			if (
				(barOpen > upperBound && barClose < lowerBound) ||
				(barOpen < lowerBound && barClose > upperBound)
			)
				level.numBodyCrosses += 1;
		}
		if (level.numTouches >= params.minTouches)
			levels.push_back(level);
	}

	sort(
		levels.begin(), levels.end(),
		[&](const auto &a, const auto &b) {
			const auto rate = [&] (const auto &level) {
				return
					level.numTouches * params.touchWeight +
					level.numBodyCrosses * params.crossWeight;
			};

			return rate(a) > rate(b);
		}
	);

	for (auto levelIt = levels.begin(); levelIt != levels.end(); ) {
		bool levelRepeat = false;
		for (auto prevLevelIt = levels.begin(); prevLevelIt != levelIt; ++prevLevelIt) {
			if (fabs(1 - prevLevelIt->level / levelIt->level) < params.sameLevelK) {
				levelRepeat = true;
				break;
			}
		}
		if (levelRepeat)
			levelIt = levels.erase(levelIt);
		else
			++levelIt;
	}

	cout << "Compacted" << endl;
	size_t n = 0;
	for (const auto & level: levels) {
		if (++n > params.maxlevels)
			break;
		cout << level.numTouches << ' ' << level.numBodyCrosses << ' ' << level.level << endl;
	}
}

}
