#pragma once

#include <iostream>
#include <optional>
#include <chart/data.h>

namespace robotrade {

struct EntryAnalyzerParams {
	double runAwayFromStopK = 2.;
	unsigned stopOnSameDayEveryNthTime = 2;
};

class EntryAnalyzer {
public:
	EntryAnalyzer(chart::data::PBars bars) : bars_(bars) {}

	struct Result {
		chart::Time orderActivated;
		chart::Price stopEnterPrice;
		chart::Price stopPrice;
		std::optional<chart::Time> stopped;

		struct Filled {
			chart::Time fillTime;
			double profitPerStopK;
			chart::Time profitTime;
		};
		std::optional<Filled> filled;
		std::optional<chart::Time> runAway;

		friend std::ostream &operator<<(std::ostream&, const Result&);
	};

	enum class Direction{Buy, Sell};

	Result analyze(
		Direction direction,
		chart::Price stopEnterPrice,
		chart::Price stopPrice,
		size_t orderBarNum
	);

private:
	chart::data::PBars bars_;
	size_t stopOnSameDayCount_ = 0;
};

}

