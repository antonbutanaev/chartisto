#pragma once

#include <optional>
#include <json/value.h>
#include <cstddef>
#include <chart/data.h>

namespace robotrade {

struct FindLevelsParams {
	double priceRangeK = 0.1;
	double precisionK = 0.001;
	double roundPrecisionK = 0.0001;
	double sameLevelK = 0.01;
	double step = 0.01;
	size_t numStepsForRound = 100;
	double tailTouchWeight = 2;
	double bodyTouchWeight = 1;
	double crossWeight = -3;
	double roundWeight = 3;
	double avgDeviationWeight = -1;
	double maxCrossRate = 0.4;
	size_t minExtremumAgeBars = 20;
	size_t minTouches = 5;
	size_t numBarsForLevel = 200;
	double levelBodyCrossPrecisionK = 0.0025;
	size_t numBarsComing = 10;
	double crossLimitOrderAboveLimitK = 0.005;
	size_t stepsForStop = 2;
	size_t stepsForEnterStop = 3;
	double losslessStopK = 2;
	double profitPerLossK = 3;

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
	double avgDeviation = 0;
};

class Levels {
public:
	Levels(const std::string &config, int daysToAnalyze, const std::string &resultFile);
	void process(chart::data::PBars bars);
private:
	FindLevelsParams getLevelsParams(const std::string &section, chart::data::PBars, size_t barFrom, size_t barTo);
	std::vector<Level> findLevels(chart::data::PBars, size_t from, size_t to);

	Json::Value config_;
	int daysToAnalyze_;
	std::ofstream result_;
};

}
