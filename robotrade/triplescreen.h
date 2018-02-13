#ifndef ROBOTRADE_TRIPLESCREEN_H_
#define ROBOTRADE_TRIPLESCREEN_H_

#include <memory>
#include <optional>
#include <chrono>
#include <vector>
#include <functional>
#include <chart/data.h>

namespace robotrade {

struct StategyResult {
	struct Trade {
		chart::Time time;
		int number;
		chart::Price enterPrice;
		chart::Price stopPrice;

		using Days = std::chrono::duration<int, std::ratio<60*60*24>>;
		std::optional<Days> daysToStop;

		chart::Price maxProfitToStop;

	};

	std::vector<Trade> trades;
};

enum class Action{Buy, Sell, Wait};

class TripleScreen {
public:
	using Criteria = std::function<Action(size_t weekly, size_t daily)>;
	TripleScreen(chart::data::PBars, chart::data::PBars, Criteria);
	~TripleScreen();

	StategyResult run();
private:
	struct Impl;
	std::unique_ptr<Impl> i_;
};

}

#endif

