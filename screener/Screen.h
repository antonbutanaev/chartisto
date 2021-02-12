#ifndef SCREENER_SCREEN_H_
#define SCREENER_SCREEN_H_

#include <iostream>
#include <string>

namespace screener {

constexpr auto Period = 7;
constexpr auto NumPeriods = 10;
constexpr auto ExtraDays = 30;
constexpr auto GoldenRatioLo = .38;
constexpr auto GoldenRatioHi = .62;

void screen(std::istream &, const std::string &quotesDir);
}

#endif
