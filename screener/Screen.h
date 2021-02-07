#ifndef SCREENER_SCREEN_H_
#define SCREENER_SCREEN_H_

#include <iostream>
#include <string>

namespace screener {

constexpr auto Period = 7;
constexpr auto NumPeriods = 7;
constexpr auto GoldRatioLo = .38;
constexpr auto GoldRatioHi = .62;

void screen(std::istream &, const std::string &quotesDir);
}

#endif
