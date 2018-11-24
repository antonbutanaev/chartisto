#pragma once

#include <random>
#include <iostream>
#include <optional>
#include <chart/data.h>

namespace robotrade {

struct EntryAnalyzerParams {
	double runAwayFromStopK = 2.;
	double losslessStopK = 2.;
};

class IProbabilityProvider {
public:
	virtual ~IProbabilityProvider() = default;
	virtual void seed(unsigned) = 0;
	virtual unsigned whatHappened(unsigned numChances) = 0;
};

class ProbabilityProvider : public IProbabilityProvider {
public:
	void seed(unsigned) override;
	unsigned whatHappened(unsigned numChances) override;
public:
	std::mt19937 rand_;
	std::uniform_real_distribution<double> dist_{0, 1};
};

class EntryAnalyzer {
public:
	EntryAnalyzer(
		const EntryAnalyzerParams &params,
		chart::data::PBars bars,
		IProbabilityProvider&,
		std::ostream&,
		unsigned seed
	);

	struct Result {
		chart::Time orderActivated;
		chart::Price stopEnterPrice;
		chart::Price stopPrice;
		chart::Price targetPrice;
		chart::Price runAwayPrice;

		struct Stopped {
			chart::Time time;
			bool lossless = false;
			bool probable = false;
		};
		std::optional<Stopped> stopped;
		std::vector<chart::Time> probablyNotStopped;

		struct Filled {
			chart::Time time;
		};
		std::optional<Filled> filled;

		struct Profit {
			chart::Time time;
			bool probable = false;
		};
		std::optional<Profit> profit;
		std::vector<chart::Time> probablyNoProfit;

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
		size_t orderBarNum
	);

private:
	EntryAnalyzerParams params_;
	chart::data::PBars bars_;
	IProbabilityProvider &probabilityProvider_;
	std::ostream &result_;
	unsigned seed_;
};

}

