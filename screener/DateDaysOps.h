#pragma once

#include <util/log.h>
#include <sstream>
#include <date/date.h>

namespace date {

inline year_month_day operator-(year_month_day a, days b) {
	return sys_days(a) - b;
}

inline year_month_day operator+(year_month_day a, days b) {
	return sys_days(a) + b;
}

inline year_month_day &operator+=(year_month_day &a, days b) {
	return a = a + b;
}

inline year_month_day &operator-=(year_month_day &a, days b) {
	return a = a - b;
}

inline year_month_day stringToDate(const std::string &dateStr) {
	std::stringstream dateStream(dateStr);
	int yi, mi, di;
	char dash1, dash2;
	dateStream >> yi >> dash1 >> mi >> dash2 >> di;
	if (!dateStream || dash1 != '-' || dash2 != '-')
		ERROR(runtime_error, "Could not parse date " << dateStr);
	return year{yi}/mi/di;
}

}
