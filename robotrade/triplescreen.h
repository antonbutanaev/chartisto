#ifndef ROBOTRADE_TRIPLESCREEN_H_
#define ROBOTRADE_TRIPLESCREEN_H_

#include <memory>
#include <chrono>
#include <vector>
#include <functional>
#include <chart/data.h>

namespace robotrade {

struct StategyResult {
	struct Trade {
		chart::Time time;
		chart::Price enterPrice;
		chart::Price stopPrice;

		using Days = std::chrono::duration<int, std::ratio<60*60*24>>;
		Days daysToStop;

		chart::Price maxProfitToStop;

	};

	std::vector<Trade> trades;
};


class TripleScreen {
public:
	using Criteria = std::function<bool(size_t weekly, size_t daily)>;
	TripleScreen(chart::data::PBars, chart::data::PBars, Criteria);
	~TripleScreen();

	StategyResult run();
private:
	struct Impl;
	std::unique_ptr<Impl> i_;
};

}

#endif

