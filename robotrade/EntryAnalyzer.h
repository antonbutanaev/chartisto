#pragma once

#include <random>
#include <iostream>
#include <optional>
#include <chart/data.h>

namespace robotrade {

struct EntryAnalyzerParams {
	double runAwayFromStopK = 2.;
};

class EntryAnalyzer {
public:
	EntryAnalyzer(chart::data::PBars bars);

	struct Result {
		chart::Time orderActivated;
		chart::Price stopEnterPrice;
		chart::Price stopPrice;
		chart::Price targetPrice;

		struct Stopped {
			chart::Time time;
			bool losslessStop;
		};
		std::optional<Stopped> stopped;

		struct Filled {
			chart::Time fillTime;
			std::optional<chart::Time> profitTime;
		};
		std::optional<Filled> filled;

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
		chart::Price targetPrice,
		size_t orderBarNum,
		unsigned seed
	);

private:
	chart::data::PBars bars_;
};

}

