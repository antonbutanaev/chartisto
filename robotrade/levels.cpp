#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <cmath>
#include <boost/io/ios_state.hpp>
#include <json/json.h>
#include <robotrade/levels.h>
#include <robotrade/entryAnalyzer.h>
#include <chart/indicators.h>

using namespace std;
using namespace chart;
using namespace boost::io;

namespace robotrade {

ostream &operator<<(ostream &o, const FindLevelsParams &params) {
	o
		<< "priceRangeK " << params.priceRangeK << endl
		<< "precisionK " << params.precisionK << endl
		<< "roundPrecisionK " << params.roundPrecisionK << endl
		<< "sameLevelK " << params.sameLevelK << endl
		<< "numStepsForRound " << params.numStepsForRound << endl
		<< "tailTouchWeight " << params.tailTouchWeight << endl
		<< "bodyTouchWeight " << params.bodyTouchWeight << endl
		<< "crossWeight " << params.crossWeight << endl
		<< "roundWeight " << params.roundWeight << endl
		<< "avgDeviationWeight " << params.avgDeviationWeight << endl
		<< "maxCrossRate " << params.maxCrossRate << endl
		<< "minExtremumAgeBars " << params.minExtremumAgeBars << endl
		<< "minTouches " << params.minTouches << endl
		<< "numBarsForLevel " << params.numBarsForLevel << endl
		<< "levelBodyCrossPrecisionK " << params.levelBodyCrossPrecisionK << endl
		<< "numBarsComing " << params.numBarsComing << endl
		<< "stepsForStop " << params.stepsForStop << endl
		<< "stepsForEnterStop " << params.stepsForEnterStop << endl
		<< "losslessStopK " << params.losslessStopK << endl
		<< "profitPerLossK " << params.profitPerLossK << endl;

	if (params.step != FindLevelsParams::NoStep)
		o << "step " << params.step << endl;

	return o;
}

FindLevelsParams Levels::getLevelsParams(
	data::PBars bars, size_t barFrom, size_t barTo
) {
	FindLevelsParams params;
	const auto extract = [&] (const Json::Value &section) {
		if (section.isMember("priceRangeK")) params.priceRangeK = section["priceRangeK"].asDouble();
		if (section.isMember("precisionK")) params.precisionK = section["precisionK"].asDouble();
		if (section.isMember("roundPrecisionK")) params.roundPrecisionK = section["roundPrecisionK"].asDouble();
		if (section.isMember("sameLevelK")) params.sameLevelK = section["sameLevelK"].asDouble();
		if (section.isMember("step")) params.step = section["step"].asDouble();
		if (section.isMember("numStepsForRound")) params.numStepsForRound = section["numStepsForRound"].asUInt();
		if (section.isMember("tailTouchWeight")) params.tailTouchWeight = section["tailTouchWeight"].asDouble();
		if (section.isMember("bodyTouchWeight")) params.bodyTouchWeight = section["bodyTouchWeight"].asDouble();
		if (section.isMember("crossWeight")) params.crossWeight = section["crossWeight"].asDouble();
		if (section.isMember("roundWeight")) params.roundWeight = section["roundWeight"].asDouble();
		if (section.isMember("avgDeviationWeight")) params.avgDeviationWeight = section["avgDeviationWeight"].asDouble();
		if (section.isMember("maxCrossRate")) params.maxCrossRate = section["maxCrossRate"].asDouble();
		if (section.isMember("minExtremumAgeBars")) params.minExtremumAgeBars = section["minExtremumAgeBars"].asUInt();
		if (section.isMember("minTouches")) params.minTouches = section["minTouches"].asUInt();
		if (section.isMember("numBarsForLevel")) params.numBarsForLevel = section["numBarsForLevel"].asUInt();
		if (section.isMember("levelBodyCrossPrecisionK")) params.levelBodyCrossPrecisionK = section["levelBodyCrossPrecisionK"].asDouble();
		if (section.isMember("numBarsComing")) params.numBarsComing = section["numBarsComing"].asUInt();
		if (section.isMember("stepsForStop")) params.stepsForStop = section["stepsForStop"].asUInt();
		if (section.isMember("stepsForEnterStop")) params.stepsForEnterStop = section["stepsForEnterStop"].asUInt();
		if (section.isMember("losslessStopK")) params.losslessStopK = section["losslessStopK"].asUInt();
		if (section.isMember("profitPerLossK")) params.profitPerLossK = section["profitPerLossK"].asDouble();
	};

	if (!config_.isMember("levels"))
		throw runtime_error("No section 'levels' in config");
	const auto levelsJson = config_["levels"];

	if (levelsJson.isMember("default"))
		extract(levelsJson["default"]);
	if (levelsJson.isMember(bars->title(0)))
		extract(levelsJson[bars->title(0)]);

	if (params.step == FindLevelsParams::NoStep) {
		for (size_t barNum = barFrom + 1; barNum < barTo; ++barNum) {
			for (const auto &a: data::Bars::PriceTypes)
				for (const auto &b: data::Bars::PriceTypes) {
					const auto updateStep = [&](size_t barNum1, size_t barNum2) {
						auto delta = fabs(bars->get(a, barNum1) - bars->get(b, barNum2));
						if (delta > PriceEpsilon)
							params.step = std::min(params.step, delta);
					};
					updateStep(barNum, barNum);
					updateStep(barNum, barNum - 1);
				}
		}
	}
	return params;
}

vector<Level> Levels::findLevels(data::PBars bars, size_t from, size_t to) {
	const auto params = getLevelsParams(bars, from, to);

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

	using chart::operator <<;
	result_
		<< "Date  range: " << bars->time(from) << " " << bars->time(to) << endl
		<< "Price range: " << rangeLow << " " << rangeHigh << endl;

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
			level.numBodyCrosses < params.maxCrossRate * (level.numBodyTouches + level.numTailTouches) &&
			level.to - level.from + 1 >= params.numBarsComing
		)
			levels.push_back(level);
	}

	// normalize and fill rates
	size_t maxNumTailTouches = 0;
	size_t maxNumBodyTouches = 0;
	double maxBodyCross = 0;
	double maxAvgDeviation = 0;
	size_t maxLength = 0;

	for (const auto &level: levels) {
		maxNumTailTouches = max(maxNumTailTouches, level.numTailTouches);
		maxNumBodyTouches = max(maxNumBodyTouches, level.numBodyTouches);
		maxBodyCross = max(maxBodyCross, level.bodyCross());
		maxAvgDeviation = max(maxAvgDeviation, level.avgDeviationPerCent);
		maxLength = max(maxLength, level.length());
	}

	for (auto &level: levels) {
		level.numTailTouchesRate = 100. * level.numTailTouches / maxNumTailTouches;
		level.numBodyTouchesRate = 100. * level.numBodyTouches / maxNumBodyTouches;
		level.bodyCrossRate = 100. * level.bodyCross() / maxBodyCross;
		level.avgDeviationRate = 100. * level.avgDeviationPerCent / maxAvgDeviation;
		level.lengthRate = 100. * level.length() / maxLength;
	}

	const auto byPrice = [&](const auto &a, const auto &b) {
		return a.level > b.level;
	};

	const auto byRate = [&](const auto &a, const auto &b) {
		const auto rate = [&] (const auto &level) {
			return
				(
					level.avgDeviationRate * params.avgDeviationWeight +
					level.numTailTouchesRate * params.tailTouchWeight +
					level.numBodyTouchesRate * params.bodyTouchWeight +
					level.bodyCrossRate * params.crossWeight
				) * (level.isRound? params.roundWeight : 1);
		};
		return rate(a) > rate(b);
	};

	const auto print = [&] (const char *tag) {
		result_
			<< tag << ": " << levels.size() << endl
			<< "TailTch\t\tBodyTch\t\tBodyX\t\tLevel\t\tExtr\tRound\tLen\t\tAvgDev%\t\tFrom\t\tTo" << endl;
		for (const auto & level: levels) {
			ios_all_saver save(result_);
			result_
				<< setprecision(3)
				<< level.numTailTouches << '\t'
				<< level.numTailTouchesRate << '\t'
				<< level.numBodyTouches << '\t'
				<< level.numBodyTouchesRate << '\t'
				<< level.numBodyCrosses << '\t'
				<< level.bodyCrossRate << '\t'
				<< setw(10) << setprecision(10) << left << level.level << '\t'
				<< setprecision(3)
				<< level.isExtrememum << '\t'
				<< level.isRound << '\t'
				<< level.length() << '\t'
				<< level.lengthRate << '\t'
				<< level.avgDeviationPerCent << '\t'
				<< level.avgDeviationRate << '\t'
				<< bars->time(level.from) << '\t'
				<< bars->time(level.to)
				<< endl;
		}
	};

	// throw away dup levels
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

	// add extremums
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


	if (!levels.empty()) {
		auto levelsByPrice = levels;
		sort(levelsByPrice.begin(), levelsByPrice.end(), byPrice);
		result_
			<< "Price\t%%" << endl
			<< levelsByPrice[0].level << endl;

		for (size_t i = 1; i < levelsByPrice.size(); ++i) {
			result_
				<< levelsByPrice[i].level << '\t'
				<< 100 * (levelsByPrice[i].level / levelsByPrice[i-1].level - 1) << endl;
		}
	}

	print("Levels compacted");
	return levels;
}

Levels::Levels(const std::string &config, unsigned daysToAnalyze, const std::string &resultFile)
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

Levels::ProcessResult Levels::process(data::PBars bars, unsigned seed) {
	ProbabilityProvider probabilityProvider;
	const auto params = getLevelsParams(bars, 0,0);
	EntryAnalyzerParams entryAnalyzerParams;
	entryAnalyzerParams.losslessStopK = params.losslessStopK;
	entryAnalyzerParams.runAwayFromStopK = params.losslessStopK;
	EntryAnalyzer entryAnalyzer(entryAnalyzerParams, bars, probabilityProvider, result_);
	vector<EntryAnalyzer::Result> results;
	result_ << "Using params:" << endl << params;
	result_ << "Using seed: " << seed << endl;

	size_t startFrom = daysToAnalyze_ == 0? 0 :  bars->num() - params.numBarsForLevel - daysToAnalyze_;
	for (
		size_t barFrom = startFrom, barTo = barFrom + params.numBarsForLevel;
		barTo < bars->num();
		++barFrom, ++barTo
	) {
		result_ << endl;

		const auto params = getLevelsParams(bars, barFrom, barTo);
		result_ << "Step " << params.step << endl;

		const auto levels = findLevels(bars, barFrom, barTo);

		const auto findResistance = [&](Price from, Price to) {
			for (const auto &level: levels) {
				if (level.level > from && level.level < to) {
					result_ << " found RESISTANCE " << level.level;
					return true;
				}
			}
			return false;
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
				const auto close = bars->close(lastBarNum);

				if (numBarsAbove == params.numBarsComing && close < crossLowerBound) {
					const auto stop = bars->low(lastBarNum) - params.stepsForStop * params.step;
					const auto enterStop = level.level + params.stepsForEnterStop * params.step;
					const auto target = enterStop + params.profitPerLossK * (enterStop - stop);
					using chart::operator <<;
					result_
						<< "CROSS DOWN level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << stop
						<< " enter " << enterStop
						<< " target " << target;

					if (!findResistance(enterStop, target)) {
						results.push_back(
							entryAnalyzer.analyze(
								EntryAnalyzer::Direction::Buy,
								enterStop,
								stop,
								target,
								lastBarNum,
								seed
							)
						);
						result_ << " " << results.back() << endl;
					}
					result_ << endl;
				}

				if (numBarsBelow == params.numBarsComing && close > crossUpperBound) {
					const auto stop = bars->high(lastBarNum) + params.stepsForStop * params.step;
					const auto enterStop = level.level - params.stepsForEnterStop * params.step;
					const auto target = enterStop - params.profitPerLossK * (stop - enterStop);
					using chart::operator <<;
					result_
						<< "CROSS UP level " << level.level
						<< " at " << bars->time(lastBarNum)
						<< " stop " << stop
						<< " enter " << enterStop
						<< " target " << target;

					if (!findResistance(target, enterStop)) {
						results.push_back(
							entryAnalyzer.analyze(
								EntryAnalyzer::Direction::Sell,
								enterStop,
								stop,
								target,
								lastBarNum,
								seed
							)
						);
						result_ << " " << results.back() << endl;
					}
					result_ << endl;
				}
			}
		}
	}

	result_ << endl << "Results:" << endl;
	ProcessResult ret{};
	ret.title = bars->title(0);
	for (const auto &result: results) {
		++ret.numOrders;
		if (result.filled && result.profit)
			++ret.numProfits;
		if (result.stopped && !result.stopped->lossless)
			++ret.numLosses;
		result_ << result << endl;
	}

	ret.finResult = -1. * ret.numLosses + params.profitPerLossK * ret.numProfits;

	result_
		<< endl
		<< "Num " << ret.numOrders << endl
		<< "Profit " << ret.numProfits << endl
		<< "Loss " << ret.numLosses << endl
		<< "Ratio " << ret.finResult
		<< endl;

	return ret;
}

}
