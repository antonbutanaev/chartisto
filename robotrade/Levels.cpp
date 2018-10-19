#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <cmath>
#include <boost/io/ios_state.hpp>
#include <json/json.h>
#include <robotrade/Levels.h>
#include <robotrade/EntryAnalyzer.h>
#include <chart/indicators.h>

using namespace std;
using namespace chart;
using namespace boost::io;

namespace robotrade {

FindLevelsParams Levels::getLevelsParams(
	const std::string &section, data::PBars bars, size_t barFrom, size_t barTo
) {
	if (!config_.isMember(section))
		throw runtime_error("No section " + section + " in config");

	const auto &sectionJson = config_[section];
	FindLevelsParams result;

	auto step = numeric_limits<Price>::max();
	for (size_t barNum = barFrom + 1; barNum < barTo; ++barNum) {
		for (const auto &a: data::Bars::PriceTypes)
			for (const auto &b: data::Bars::PriceTypes) {
				const auto updateStep = [&](size_t barNum1, size_t barNum2) {
					auto delta = fabs(bars->get(a, barNum1) - bars->get(b, barNum2));
					if (delta > PriceEpsilon)
						step = std::min(step, delta);
				};
				updateStep(barNum, barNum);
				updateStep(barNum, barNum - 1);
			}
	}
	result.step = step;
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
	const auto params = getLevelsParams("default", bars, from, to);

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

	result_
		<< "Date  range: " << bars->time(from) << " " << bars->time(to) << endl
		<< "Price range: " << rangeLow << " " << rangeHigh << endl;

	if (
		minPrice.price >= rangeLow &&
		minPrice.barNum + params.minExtremumAgeBars < to &&
		minPrice.barNum - params.minExtremumAgeBars >= from
	) {
		Level level;
		level.level = minPrice.price;
		level.isExtrememum = true;
		level.from = minPrice.barNum;
		level.to = minPrice.barNum;
		levels.push_back(level);
	}

	if (
		maxPrice.price <= rangeHigh &&
		maxPrice.barNum + params.minExtremumAgeBars < to &&
		maxPrice.barNum - params.minExtremumAgeBars >= from
	) {
		Level level;
		level.level = maxPrice.price;
		level.isExtrememum = true;
		level.from = maxPrice.barNum;
		level.to = maxPrice.barNum;
		levels.push_back(level);
	}

	for (auto price = rangeLow; price <= rangeHigh; price += params.step) {
		Level level;
		level.level = price;
		level.from = to;
		level.to = from;

		for (size_t barNum = from; barNum < to; ++barNum) {
			const auto upperBound = price * (1 + params.precisionK);
			const auto lowerBound = price * (1 - params.precisionK);

			for (const auto &priceType: data::Bars::PriceTypes) {
				const auto barPrice = bars->get(priceType, barNum);
				if (barPrice >= lowerBound && barPrice <= upperBound) {
					level.avgDeviationPerCent += fabs(barPrice - price);
					(
						priceType == data::Bars::PriceType::High ||
						priceType == data::Bars::PriceType::Low?
							level.numTailTouches : level.numBodyTouches
					) += 1;
					level.from = min(level.from, barNum);
					level.to = max(level.to, barNum);
				}
			}

			const auto barOpen = bars->open(barNum);
			const auto barClose = bars->close(barNum);
			if (
				(barOpen > upperBound && barClose < lowerBound) ||
				(barOpen < lowerBound && barClose > upperBound)
			) {
				level.numBodyCrosses += 1;
				level.from = min(level.from, barNum);
				level.to = max(level.to, barNum);
			}
		}

		if (level.numTailTouches + level.numBodyTouches) {
			level.avgDeviationPerCent /= level.numTailTouches + level.numBodyTouches;
			level.avgDeviationPerCent = 100 * level.avgDeviationPerCent / level.level;
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
					level.avgDeviationPerCent * params.avgDeviationWeight +
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
		result_
			<< tag << ": " << levels.size() << endl
			<< "TailTch\tBodyTch\tBodyX\tLevel\t\tExtr\tRound\tLen\tAvgDev%\tFrom\t\tTo" << endl;
		for (const auto & level: levels) {
			ios_all_saver save(result_);
			result_
				<< level.numTailTouches << '\t'
				<< level.numBodyTouches << '\t'
				<< level.numBodyCrosses << '\t'
				<< setw(10) << left << level.level << '\t'
				<< level.isExtrememum << '\t'
				<< level.isRound << '\t'
				<< level.to - level.from << '\t'
				<< fixed << setprecision(3) << level.avgDeviationPerCent << '\t'
				<< bars->time(level.from) << '\t'
				<< bars->time(level.to)
				<< endl;
		}
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

Levels::Levels(const std::string &config, int daysToAnalyze, const std::string &resultFile)
:
	daysToAnalyze_(daysToAnalyze),
	result_(resultFile)
{
	result_ << "Processing " << resultFile << " using config " << config << endl;
	ifstream ifs(config);
	if (!ifs)
		throw runtime_error("Could not read config " + config);
	Json::Value configJson;
	ifs >> config_;
}

void Levels::process(data::PBars bars) {
	EntryAnalyzer entryAnalyzer(bars);
	vector<EntryAnalyzer::Result> results;
	const auto params = getLevelsParams("default", bars, 0,0);
	size_t startFrom = daysToAnalyze_ == 0? 0 :  bars->num() - params.numBarsForLevel - daysToAnalyze_;
	for (
		size_t barFrom = startFrom, barTo = barFrom + params.numBarsForLevel;
		barTo < bars->num();
		++barFrom, ++barTo
	) {
		result_ << endl;

		const auto params = getLevelsParams("default", bars, barFrom, barTo);
		result_ << "Step " << params.step << endl;

		const auto levels = findLevels(bars, barFrom, barTo);

		const auto findLevels = [&](Price from, Price to) {
			bool found = false;
			for (const auto &level: levels) {
				if (level.level > from && level.level < to) {
					if (!found)
						result_ << "Found levels between enter and target:";
					found = true;
					result_ << ' ' << level.level;
				}
			}
			if (found)
				result_ << endl;
			return found;
		};

		for (const auto &level: levels) {
			const auto upperBound = level.level * (1 + params.precisionK);
			const auto lowerBound = level.level * (1 - params.precisionK);
			size_t numBarsBelow = 0;
			size_t numBarsAbove = 0;
			auto lastBarNum = barTo;
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
					const auto stop = bars->close(lastBarNum) - params.stepsForStop * params.step;
					const auto enterStop = level.level + params.stepsForEnterStop * params.step;
					const auto target = enterStop + params.profitPerLossK * (enterStop - stop);
					result_
						<< "CROSS DOWN level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << stop
						<< " enter " << enterStop
						<< " target " << target
						<< endl;

					if (!findLevels(enterStop, target)) {
						results.push_back(
							entryAnalyzer.analyze(
								EntryAnalyzer::Direction::Buy,
								enterStop,
								stop,
								lastBarNum
							)
						);
						result_ << "Result " << results.back() << endl;
					}
					break;
				}

				if (numBarsBelow == params.numBarsComing && open < crossLowerBound && close > crossUpperBound) {
					const auto stop = bars->close(lastBarNum) + params.stepsForStop * params.step;
					const auto enterStop = level.level - params.stepsForEnterStop * params.step;
					const auto target = enterStop - params.profitPerLossK * (stop - enterStop);
					result_
						<< "CROSS UP level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << stop
						<< " enter " << enterStop
						<< " target " << target
						<< endl;

					if (!findLevels(target, enterStop)) {
						results.push_back(
							entryAnalyzer.analyze(
								EntryAnalyzer::Direction::Sell,
								enterStop,
								stop,
								lastBarNum
							)
						);
						result_ << "Result " << results.back() << endl;
					}
					break;
				}
			}
		}
	}

	if (results.empty())
		return;

	result_ << endl << "Results:" << endl;
	size_t num = 0;
	size_t numProfit = 0;
	size_t numLoss = 0;
	for (const auto &result: results) {
		++num;
		if (result.filled && result.filled->profitPerStopK > params.profitPerLossK)
			++numProfit;
		if (result.filled && result.filled->profitPerStopK < params.losslessStopK)
			++numLoss;
		result_ << result << endl;
	}
	result_
		<< endl
		<< "Num " << num << endl
		<< "Profit " << numProfit << endl
		<< "Loss " << numLoss << endl
		<< "Ratio " << -1. * numLoss + params.profitPerLossK * numProfit
		<< endl;

}

}
