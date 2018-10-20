#pragma once

#include <random>
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
	EntryAnalyzer(chart::data::PBars bars);

	struct Result {
		chart::Time orderActivated;
		chart::Price stopEnterPrice;
		chart::Price stopPrice;
		struct Stopped {
			chart::Time time;
		};
		std::optional<Stopped> stopped;
		std::optional<bool> onSameDayStopped;

		struct Filled {
			chart::Time fillTime;
			double profitPerStopK;
			chart::Time profitTime;
		};
		std::optional<Filled> filled;
		std::optional<bool> onStopDayProfit;

		struct RanAway {
			chart::Time time;
			chart::Price price;
		};
		std::optional<RanAway> runAway;

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
	std::mt19937 rand_;
};

}

