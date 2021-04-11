#pragma once

#include <optional>
#include <iostream>
#include <string>

#include "Calcs.h"

namespace screener {

const auto RelativeVolumePeriod = date::days{30};
const auto RelativeChangePeriod = date::days{90};
constexpr auto Period = 7;
constexpr auto NumPeriods = 13;
constexpr auto ExtraDays = 30;
constexpr auto GoldenRatioHi = .6180339887;
constexpr auto GoldenRatioLo = 1 - GoldenRatioHi;

struct ScreenParams {
	Date toDate;
	double accelerationRate;
};

void screen(std::istream &, const std::string &quotesDir, const ScreenParams&);

}
