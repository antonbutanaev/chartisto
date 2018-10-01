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
	double priceRangeK = 0.2;
	size_t minTouches = 3;
	Price precisionK = 0.001;
	Price step = 0.01;
	size_t numStepsForRound = 100;
	double tailTouchWeight = 2;
	double bodyTouchWeight = 1;
	double crossWeight = -4;
	double roundWeight = 3;
	double sameLevelK = 0.03;
	size_t maxLevels = 10;
	size_t minExtremumAgeBars = 20;
	size_t extremumNumTouches = 40;
};

struct Level {
	size_t numTailTouches;
	size_t numBodyTouches;
	size_t numBodyCrosses;
	Price level;
};

FindLevelsParams getLevelsParams(const Json::Value &config, const std::string &section) {
	if (!config.isMember(section))
		throw runtime_error("No section " + section + " in config");

	const auto &sectionJson = config[section];
	FindLevelsParams result;

	if (sectionJson.isMember("priceRangeK"))
		result.priceRangeK = sectionJson["priceRangeK"].asDouble();

	if (sectionJson.isMember("minTouches"))
		result.minTouches = sectionJson["minTouches"].asUInt();

	if (sectionJson.isMember("precisionK"))
		result.precisionK = sectionJson["precisionK"].asDouble();

	if (sectionJson.isMember("step"))
		result.step = sectionJson["step"].asDouble();

	if (sectionJson.isMember("step"))
		result.step = sectionJson["step"].asDouble();

	if (sectionJson.isMember("tailTouchWeight"))
		result.tailTouchWeight = sectionJson["tailTouchWeight"].asDouble();

	if (sectionJson.isMember("bodyTouchWeight"))
		result.bodyTouchWeight = sectionJson["bodyTouchWeight"].asDouble();

	if (sectionJson.isMember("crossWeight"))
		result.crossWeight = sectionJson["crossWeight"].asDouble();

	if (sectionJson.isMember("sameLevelK"))
		result.sameLevelK = sectionJson["sameLevelK"].asDouble();

	if (sectionJson.isMember("maxLevels"))
		result.maxLevels = sectionJson["maxLevels"].asUInt();

	if (sectionJson.isMember("minExtremumAgeBars"))
		result.minExtremumAgeBars = sectionJson["minExtremumAgeBars"].asUInt();

	if (sectionJson.isMember("extremumNumTouches"))
		result.extremumNumTouches = sectionJson["extremumNumTouches"].asUInt();

	return result;
}

void findLevels(data::PBars bars, size_t from, size_t to, const std::string &config) {
	cout << "Using config " << config << endl;
	ifstream ifs(config);
	if (!ifs)
		throw runtime_error("Could not read config " + config);
	Json::Value configJson;
	ifs >> configJson;

	const auto params = getLevelsParams(configJson, "default");

	std::vector<Level> levels;
	if (from >= to)
		return;

	struct ExtremumPrice {
		Price price;
		size_t barNum;
	};

	ExtremumPrice minTailPrice{numeric_limits<Price>::max(), to};
	ExtremumPrice maxTailPrice{numeric_limits<Price>::lowest(), to};

	for (auto barNum = from; barNum < to; ++barNum) {
		if (minTailPrice.price > bars->low(barNum)) {
			minTailPrice.price = bars->low(barNum);
			minTailPrice.barNum = barNum;
		}
		if (maxTailPrice.price < bars->high(barNum)) {
			maxTailPrice.price = bars->high(barNum);
			maxTailPrice.barNum = barNum;
		}
	}

	auto rangeLow = bars->close(to - 1) * (1 - params.priceRangeK);
	auto rangeHigh = bars->close(to - 1) * (1 + params.priceRangeK);
	rangeLow = ceil(rangeLow / params.step) * params.step;
	rangeHigh = floor(rangeHigh / params.step) * params.step;

	rangeLow = std::max(minTailPrice.price, rangeLow);
	rangeHigh = std::min(maxTailPrice.price, rangeHigh);

	cout << "Price range: " << rangeLow << " " << rangeHigh << endl;

	if (minTailPrice.price >= rangeLow && minTailPrice.barNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			0,
			minTailPrice.price
		});

	if (maxTailPrice.price <= rangeHigh && maxTailPrice.barNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			0,
			maxTailPrice.price
		});

	for (auto price = rangeLow; price <= rangeHigh; price += params.step) {
		Level level{0, 0, 0, price};
		for (size_t barNum = from; barNum < to; ++barNum) {
			const auto upperBound = price * (1 + params.precisionK);
			const auto lowerBound = price * (1 - params.precisionK);

			for (const auto &priceType: data::Bars::PriceTypes) {
				const auto barPrice = bars->get(priceType, barNum);
				if (barPrice >= lowerBound && barPrice <= upperBound) {
					(priceType == data::Bars::PriceType::Open || priceType == data::Bars::PriceType::Close?
						level.numBodyTouches : level.numTailTouches
					) += 1;
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


		cout << "Lev\t"
			<< level.numTailTouches << '\t'
			<< level.numBodyTouches << '\t'
			<< level.numBodyCrosses << '\t'
			<< level.level << endl;


		if (level.numBodyTouches + level.numTailTouches >= params.minTouches)
			levels.push_back(level);
	}

	sort(
		levels.begin(), levels.end(),
		[&](const auto &a, const auto &b) {
			const auto rate = [&] (const auto &level) {
				const auto k = params.step * params.numStepsForRound;
				const auto roundPrice = round(level.level / k) * k;
				auto roundWeight = params.roundWeight;
				if (fabs(level.level / roundPrice - 1) > params.precisionK)
					roundWeight = 1;

				return
					roundWeight * (
						level.numTailTouches * params.tailTouchWeight +
						level.numBodyTouches * params.bodyTouchWeight
					) +
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

	cout << "numTailTouches\tnumBodyTouches\tnumBodyCrosses\tlevel" << endl;
	size_t n = 0;
	for (const auto & level: levels) {
		if (++n > params.maxLevels)
			break;
		cout
			<< level.numTailTouches << '\t'
			<< level.numBodyTouches << '\t'
			<< level.numBodyCrosses << '\t'
			<< level.level << endl;
	}
}

}
