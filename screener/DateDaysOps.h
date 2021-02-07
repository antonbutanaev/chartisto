#pragma once

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

}
