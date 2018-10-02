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
	Price precisionK = 0.001;
	double sameLevelK = 0.01;
	Price step = 0.01;
	size_t numStepsForRound = 100;
	double tailTouchWeight = 2;
	double bodyTouchWeight = 1;
	double crossWeight = -3;
	double roundWeight = 3;
	double nearExtremumWeight = 4;
	double maxCrossRate = 0.5;
	size_t minExtremumAgeBars = 20;
	size_t minTouches = 3;
};

struct Level {
	size_t numTailTouches;
	size_t numBodyTouches;
	size_t numBodyCrosses;
	Price level;
	bool isExtrememum = false;
	double powerK = 1;
};

FindLevelsParams getLevelsParams(const Json::Value &config, const std::string &section) {
	if (!config.isMember(section))
		throw runtime_error("No section " + section + " in config");

	const auto &sectionJson = config[section];
	FindLevelsParams result;

	if (sectionJson.isMember("priceRangeK"))
		result.priceRangeK = sectionJson["priceRangeK"].asDouble();
	if (sectionJson.isMember("precisionK"))
		result.precisionK = sectionJson["precisionK"].asDouble();
	if (sectionJson.isMember("sameLevelK"))
		result.sameLevelK = sectionJson["sameLevelK"].asDouble();
	if (sectionJson.isMember("step"))
		result.step = sectionJson["step"].asDouble();
	if (sectionJson.isMember("numStepsForRound"))
		result.numStepsForRound = sectionJson["numStepsForRound"].asUInt();
	if (sectionJson.isMember("tailTouchWeight"))
		result.tailTouchWeight = sectionJson["tailTouchWeight"].asDouble();
	if (sectionJson.isMember("bodyTouchWeight"))
		result.bodyTouchWeight = sectionJson["bodyTouchWeight"].asDouble();
	if (sectionJson.isMember("crossWeight"))
		result.crossWeight = sectionJson["crossWeight"].asDouble();
	if (sectionJson.isMember("roundWeight"))
		result.roundWeight = sectionJson["roundWeight"].asDouble();
	if (sectionJson.isMember("nearExtremumWeight"))
		result.nearExtremumWeight = sectionJson["nearExtremumWeight"].asDouble();
	if (sectionJson.isMember("maxCrossRate"))
		result.maxCrossRate = sectionJson["maxCrossRate"].asDouble();
	if (sectionJson.isMember("minExtremumAgeBars"))
		result.minExtremumAgeBars = sectionJson["minExtremumAgeBars"].asUInt();
	if (sectionJson.isMember("minTouches"))
		result.minTouches = sectionJson["minTouches"].asUInt();

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
		size_t barNum = 0;
	};

	ExtremumPrice minPrice{numeric_limits<Price>::max()};
	ExtremumPrice maxPrice{numeric_limits<Price>::lowest()};

	for (auto barNum = from; barNum < to; ++barNum) {
		if (minPrice.price > bars->low(barNum)) {
			minPrice.price = bars->low(barNum);
			minPrice.barNum = barNum;
		}
		if (maxPrice.price < bars->high(barNum)) {
			maxPrice.price = bars->high(barNum);
			maxPrice.barNum = barNum;
		}
	}

	auto rangeLow = bars->close(to - 1) * (1 - params.priceRangeK);
	auto rangeHigh = bars->close(to - 1) * (1 + params.priceRangeK);
	rangeLow = ceil(rangeLow / params.step) * params.step;
	rangeHigh = floor(rangeHigh / params.step) * params.step;

	rangeLow = std::max(minPrice.price, rangeLow);
	rangeHigh = std::min(maxPrice.price, rangeHigh);

	cout << "Price range: " << rangeLow << " " << rangeHigh << endl;

	if (
		minPrice.price >= rangeLow &&
		minPrice.barNum + params.minExtremumAgeBars <= to &&
		minPrice.barNum - params.minExtremumAgeBars >= from
	)
		levels.push_back({0, 0, 0, minPrice.price, true});

	if (
		maxPrice.price <= rangeHigh &&
		maxPrice.barNum + params.minExtremumAgeBars <= to &&
		maxPrice.barNum + params.minExtremumAgeBars >= from
	)
		levels.push_back({0, 0, 0, maxPrice.price, true});

	for (auto price = rangeLow; price <= rangeHigh; price += params.step) {
		Level level{0, 0, 0, price};
		for (size_t barNum = from; barNum < to; ++barNum) {
			const auto upperBound = price * (1 + params.precisionK);
			const auto lowerBound = price * (1 - params.precisionK);

			for (const auto &priceType: data::Bars::PriceTypes) {
				const auto barPrice = bars->get(priceType, barNum);
				if (barPrice >= lowerBound && barPrice <= upperBound) {
					(
						priceType == data::Bars::PriceType::High ||
						priceType == data::Bars::PriceType::Low?
							level.numTailTouches : level.numBodyTouches
					) += 1;
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

		const auto roundK = params.step * params.numStepsForRound;
		const auto roundPrice = round(price / roundK) * roundK;
		if (fabs(level.level / roundPrice - 1) < params.precisionK)
			level.powerK *= params.roundWeight;

		if (
			level.numBodyTouches + level.numTailTouches >= params.minTouches &&
			level.numBodyCrosses < params.maxCrossRate * (level.numBodyTouches + level.numTailTouches)
		)
			levels.push_back(level);
	}

	const auto byPrice = [](const auto &a, const auto &b) {
		return a.level < b.level;
	};

	const auto byRate = [&](const auto &a, const auto &b) {
		const auto rate = [&] (const auto &level) {
			return
				(
					level.numTailTouches * params.tailTouchWeight +
					level.numBodyTouches * params.bodyTouchWeight +
					level.numBodyCrosses * params.crossWeight
				) * level.powerK;
		};

		return
			a.isExtrememum != b.isExtrememum?
				a.isExtrememum > b.isExtrememum : rate(a) > rate(b);
	};

	const auto print = [&] (const char *tag) {
		cout << tag << ": " << levels.size() << endl;
		cout << "numTailTouches\tnumBodyTouches\tnumBodyCrosses\tlevel\tk\tisExtremum" << endl;
		for (const auto & level: levels)
			cout
				<< level.numTailTouches << '\t'
				<< level.numBodyTouches << '\t'
				<< level.numBodyCrosses << '\t'
				<< level.level << '\t'
				<< level.powerK << '\t'
				<< level.isExtrememum
				<< endl;
	};

	sort(levels.begin(), levels.end(), byRate);
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

	sort(levels.begin(), levels.end(), byPrice);
	if (levels.size() > 1) {
		if (levels.front().isExtrememum)
			levels.begin()[1].powerK *= params.nearExtremumWeight;
		if (levels.back().isExtrememum)
			levels.begin()[-2].powerK *= params.nearExtremumWeight;
	}

	sort(levels.begin(), levels.end(), byRate);
	print("Compacted");
}

}
