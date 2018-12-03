#pragma once

#include <chart/data.h>

namespace Json {class Value;}

namespace robotrade {

chart::data::PBars parseWithToday(
	const std::string &quotes,
	const std::string &todayQuotesDir,
	const Json::Value &titleToTickers
);

}
