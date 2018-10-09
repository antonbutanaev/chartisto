#pragma once

#include <optional>
#include <chart/data.h>

namespace robotrade {

class EntryAnalyzer {
public:
	EntryAnalyzer(chart::data::PBars bars) : bars_(bars) {}

	struct Result {
		size_t numBars;
		std::optional<double> profitK;
	};

	enum class Direction{Buy, Sell};

	std::optional<Result> analyze(
		Direction direction,
		chart::Price limitOrderPrice,
		chart::Price stopPrice,
		size_t orderBarNum
	);

private:
	chart::data::PBars bars_;
};

}

