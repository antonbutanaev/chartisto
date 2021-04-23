#pragma once

#include <optional>
#include <iostream>
#include <string>

#include "Calcs.h"

namespace screener {

constexpr auto RelativeVolumePeriod = date::days{30};
constexpr auto RelativeChangePeriod = date::days{90};
constexpr auto StableGrowthPeriod = 4;
constexpr auto Period = 7;
constexpr auto NumPeriods = 13;
constexpr auto ExtraDays = 30;
constexpr auto WeightPowerRate = 1.5;

struct ScreenParams {
	Date toDate;
	double accelerationRate;
};

void screen(std::istream &, const std::string &quotesDir, const ScreenParams&);

}
