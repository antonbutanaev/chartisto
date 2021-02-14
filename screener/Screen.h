#pragma once

#include <optional>
#include <iostream>
#include <string>

#include "Util.h"

namespace screener {

constexpr auto Period = 7;
constexpr auto NumPeriods = 10;
constexpr auto ExtraDays = 30;
constexpr auto GoldenRatioLo = .38;
constexpr auto GoldenRatioHi = 1. - GoldenRatioLo;

struct ScreenParams {
	std::optional<Date> toDate;
	double accelerationRate;
};

void screen(std::istream &, const std::string &quotesDir, const ScreenParams&);

}
