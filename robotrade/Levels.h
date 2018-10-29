#pragma once

#include <fstream>
#include <limits>
#include <optional>
#include <json/value.h>
#include <cstddef>
#include <chart/data.h>

namespace robotrade {

struct FindLevelsParams {
	static constexpr double NoStep = std::numeric_limits<double>::max();

	double priceRangeK = 0.1;
	double precisionK = 0.0025;
	double roundPrecisionK = 0.0001;
	double sameLevelK = 0.025;
	double step = NoStep;
	size_t numStepsForRound = 100;
	double tailTouchWeight = 1;
	double bodyTouchWeight = 1;
	double crossWeight = -1;
	double roundWeight = 0.1;
	double avgDeviationWeight = -1;
	double maxCrossRate = 0.3;
	size_t minExtremumAgeBars = 20;
	size_t minTouches = 5;
	size_t numBarsForLevel = 200;
	double levelBodyCrossPrecisionK = 0.0025;
	size_t numBarsComing = 10;
	size_t stepsForStop = 2;
	size_t stepsForEnterStop = 3;
	double losslessStopK = 2;
	double profitPerLossK = 3;

	friend std::ostream &operator<<(std::ostream&, const FindLevelsParams&);
};

struct Level {
	size_t numTailTouches = 0;
	size_t numBodyTouches = 0;
	size_t numBodyCrosses = 0;
	chart::Price level = 0;
	bool isExtrememum = false;
	bool isRound = false;
	size_t from = 0;
	size_t to = 0;
	double avgDeviationPerCent = 0;

	double numTailTouchesRate = 0;
	double numBodyTouchesRate = 0;
	double bodyCrossRate = 0;
	double avgDeviationRate = 0;
	double lengthRate = 0;

	size_t length() const {return to - from + 1;}
	double bodyCross() const {
		const auto numTouches = numTailTouches + numBodyTouches;
		return numTouches == 0? 0. : 1. * numBodyCrosses / numTouches;
	}
};

class Levels {
public:
	Levels(const std::string &config, unsigned daysToAnalyze, const std::string &resultFile);

	struct ProcessResult {
		std::string title;
		size_t numOrders;
		size_t numLosses;
		size_t numProfits;
		double finResult;
	};
	ProcessResult process(chart::data::PBars bars, unsigned seed);
private:
	FindLevelsParams getLevelsParams(chart::data::PBars, size_t barFrom, size_t barTo);
	std::vector<Level> findLevels(chart::data::PBars, size_t from, size_t to);

	Json::Value config_;
	unsigned daysToAnalyze_;
	std::ofstream result_;
};

}
