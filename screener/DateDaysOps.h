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
	char dash;
	dateStream >> yi >> dash >> mi >> dash >> di;
	if (!dateStream)
		ERROR(runtime_error, "could not parse date " << dateStr);
	return year{yi}/mi/di;
}

}
