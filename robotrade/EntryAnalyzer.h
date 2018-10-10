#pragma once

#include <iostream>
#include <optional>
#include <chart/data.h>

namespace robotrade {

class EntryAnalyzer {
public:
	EntryAnalyzer(chart::data::PBars bars) : bars_(bars) {}

	struct Result {
		struct Filled {
			chart::Price fillPrice;
			chart::Time time;
		};
		std::optional<Filled> filled;

		struct Stopped {
			chart::Price stopPrice;
			chart::Time time;
		};
		std::optional<Stopped> stopped;

		struct Profit {
			double profitPerStopK;
			chart::Time time;
		};
		std::optional<Profit> profit;

		friend std::ostream &operator<<(std::ostream&, const Result&);
	};

	enum class Direction{Buy, Sell};

	Result analyze(
		Direction direction,
		chart::Price limitOrderPrice,
		chart::Price stopPrice,
		size_t orderBarNum
	);

private:
	chart::data::PBars bars_;
};

}

