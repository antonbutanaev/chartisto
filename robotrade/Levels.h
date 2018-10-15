#pragma once

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
	double maxCrossRate = 0.4;
	size_t minExtremumAgeBars = 20;
	size_t minTouches = 3;
	size_t numBarsForLevel = 400;
	double levelBodyCrossPrecisionK = 0.005;
	size_t numBarsComing = 5;
	double crossLimitOrderAboveLimitK = 0.005;
};

struct Level {
	size_t numTailTouches;
	size_t numBodyTouches;
	size_t numBodyCrosses;
	chart::Price level;
	bool isExtrememum = false;
	bool isRound = false;
};

class Levels {
public:
	Levels(const std::string &config, const std::string &resultFile);
	void process(chart::data::PBars bars);
private:
	FindLevelsParams getLevelsParams(const std::string &section, chart::data::PBars);
	std::vector<Level> findLevels(chart::data::PBars, size_t from, size_t to);

	Json::Value config_;
	std::ofstream result_;
};

}
