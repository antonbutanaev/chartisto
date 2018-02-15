#ifndef ROBOTRADE_TRIPLESCREEN_H_
#define ROBOTRADE_TRIPLESCREEN_H_

#include <memory>
#include <limits>
#include <chrono>
#include <vector>
#include <functional>
#include <chart/data.h>

namespace robotrade {

struct StrategyResult {
	struct Trade {
		chart::Time time = chart::NoTime;
		chart::Time stopTime = chart::NoTime;
		int number = 0;
		chart::Price enterPrice = chart::NoPrice;
		chart::Price stopPrice = chart::NoPrice;
		static constexpr auto NoStop = std::numeric_limits<int>::max();
		int barsToStop = NoStop;
		chart::Price maxProfitToStop = chart::NoPrice;
	};

	std::vector<Trade> trades;
};

enum class Action{Buy, Sell, Wait};

class TripleScreen {
public:
	using Criteria = std::function<Action(size_t weekly, size_t daily)>;
	TripleScreen(chart::data::PBars, chart::data::PBars, Criteria);
	~TripleScreen();

	StrategyResult run();
private:
	struct Impl;
	std::unique_ptr<Impl> i_;
};

}

#endif

