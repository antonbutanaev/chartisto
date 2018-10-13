#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>
#include <json/json.h>
#include <robotrade/Levels.h>
#include <robotrade/EntryAnalyzer.h>

using namespace std;
using namespace chart;

namespace robotrade {

FindLevelsParams Levels::getLevelsParams(const std::string &section) {
	if (!config_.isMember(section))
		throw runtime_error("No section " + section + " in config");

	const auto &sectionJson = config_[section];
	FindLevelsParams result;
	return result;

	if (sectionJson.isMember("priceRangeK"))
		result.priceRangeK = sectionJson["priceRangeK"].asDouble();
	if (sectionJson.isMember("precisionK"))
		result.precisionK = sectionJson["precisionK"].asDouble();
	if (sectionJson.isMember("roundPrecisionK"))
		result.roundPrecisionK = sectionJson["roundPrecisionK"].asDouble();
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
	if (sectionJson.isMember("maxCrossRate"))
		result.maxCrossRate = sectionJson["maxCrossRate"].asDouble();
	if (sectionJson.isMember("minExtremumAgeBars"))
		result.minExtremumAgeBars = sectionJson["minExtremumAgeBars"].asUInt();
	if (sectionJson.isMember("minTouches"))
		result.minTouches = sectionJson["minTouches"].asUInt();

	return result;
}

vector<Level> Levels::findLevels(data::PBars bars, size_t from, size_t to) {
	const auto params = getLevelsParams("default");

	std::vector<Level> levels;
	if (from >= to)
		return {};

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

	cout << "Price range: " << bars->time(from) << " " << bars->time(to) << endl;
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
		if (fabs(level.level / roundPrice - 1) < params.roundPrecisionK)
			level.isRound = 1;

		if (
			level.numBodyTouches + level.numTailTouches >= params.minTouches &&
			level.numBodyCrosses < params.maxCrossRate * (level.numBodyTouches + level.numTailTouches)
		)
			levels.push_back(level);
	}

	const auto byRate = [&](const auto &a, const auto &b) {
		const auto rate = [&] (const auto &level) {
			return
				(
					level.numTailTouches * params.tailTouchWeight +
					level.numBodyTouches * params.bodyTouchWeight +
					level.numBodyCrosses * params.crossWeight
				) * (level.isRound? params.roundWeight : 1);
		};

		return
			a.isExtrememum != b.isExtrememum?
				a.isExtrememum > b.isExtrememum : rate(a) > rate(b);
	};

	const auto print = [&] (const char *tag) {
		cout << tag << ": " << levels.size() << endl;
		cout << "numTailTouches\tnumBodyTouches\tnumBodyCrosses\tlevel\tisExtremum\tisRound" << endl;
		for (const auto & level: levels)
			cout
				<< level.numTailTouches << '\t'
				<< level.numBodyTouches << '\t'
				<< level.numBodyCrosses << '\t'
				<< level.level << '\t'
				<< level.isExtrememum << '\t'
				<< level.isRound
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

	sort(levels.begin(), levels.end(), byRate);
	print("Levels compacted");
	return levels;
}

Levels::Levels(const std::string &config) {
	cout << "Using config " << config << endl;
	ifstream ifs(config);
	if (!ifs)
		throw runtime_error("Could not read config " + config);
	Json::Value configJson;
	ifs >> config_;
}

void Levels::process(chart::data::PBars bars) {
	EntryAnalyzer entryAnalyzer(bars);
	vector<EntryAnalyzer::Result> results;
	const auto params = getLevelsParams("default");
	for (size_t barFrom = 0, barTo = params.numBarsForLevel; barTo < bars->num(); ++barFrom, ++barTo) {
		const auto levels = findLevels(bars, barFrom, barTo);
		for (const auto &level: levels) {
			const auto upperBound = level.level * (1 + params.precisionK);
			const auto lowerBound = level.level * (1 - params.precisionK);
			size_t numBarsBelow = 0;
			size_t numBarsAbove = 0;
			auto lastBarNum = barTo - 1;
			for (auto barNum = lastBarNum - params.numBarsComing; barNum < lastBarNum; ++barNum) {
				const auto open = bars->open(barNum);
				const auto close = bars->close(barNum);
				if (open < upperBound && close < upperBound)
					++numBarsBelow;
				if (open > lowerBound && close > lowerBound)
					++numBarsAbove;
			}
			if (
				numBarsBelow == params.numBarsComing ||
				numBarsAbove == params.numBarsComing
			) {
				const auto crossUpperBound = level.level * (1 + params.levelBodyCrossPrecisionK);
				const auto crossLowerBound = level.level * (1 - params.levelBodyCrossPrecisionK);
				const auto open = bars->open(lastBarNum);
				const auto close = bars->close(lastBarNum);

				if (numBarsAbove == params.numBarsComing && open > crossUpperBound && close < crossLowerBound) {
					results.push_back(
						entryAnalyzer.analyze(
							EntryAnalyzer::Direction::Buy,
							level.level * (1 + params.levelBodyCrossPrecisionK),
							close,
							lastBarNum
						)
					);

					cout
						<< "CROSS DOWN level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << close
						<< endl
						<< "Result " << results.back()
						<< endl;
					break;
				}

				if (numBarsBelow == params.numBarsComing && open < crossLowerBound && close > crossUpperBound) {
					results.push_back(
						entryAnalyzer.analyze(
							EntryAnalyzer::Direction::Sell,
							level.level * (1 - params.levelBodyCrossPrecisionK),
							close,
							lastBarNum
						)
					);

					cout
						<< "CROSS UP level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << close
						<< endl
						<< "Result " << results.back()
						<< endl;
					break;
				}
			}
		}
	}

	cout << "Results:" << endl;
	for (const auto &result: results) {
		cout << result << endl;
	}
}

}
