#pragma once

#include <optional>
#include <iostream>
#include <string>

#include "Util.h"

namespace screener {

constexpr auto Period = 7;
constexpr auto NumPeriods = 10;
constexpr auto ExtraDays = 30;
constexpr auto GoldenRatioHi = .6180339887;
constexpr auto GoldenRatioLo = 1 - GoldenRatioHi;

struct ScreenParams {
	Date toDate;
	double accelerationRate;
};

void screen(std::istream &, const std::string &quotesDir, const ScreenParams&);

}
