#pragma once

#include <optional>
#include <chart/data.h>

namespace robotrade {

class EntryAnalyzer {
public:
	EntryAnalyzer(chart::data::PBars bars) : bars_(bars) {}

	struct Result {
		double profitK;
		size_t numBars;
	};

	enum class Direction{Buy, Sell};

	std::optional<Result> analyze(Direction direction, chart::Price limitOrderPrice);

private:
	chart::data::PBars bars_;
};

}

