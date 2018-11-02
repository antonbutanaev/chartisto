#ifndef ROBOTRADE_TRIPLESCREEN_H_
#define ROBOTRADE_TRIPLESCREEN_H_

#include <memory>
#include <limits>
#include <chrono>
#include <vector>
#include <functional>
#include <chart/data.h>
#include <robotrade/trader.h>

namespace robotrade {

enum class Action{Buy, Sell, Wait};

class TripleScreen {
public:
	using Criteria = std::function<Action(size_t weekly, size_t daily)>;
	TripleScreen(chart::data::PBars, chart::data::PBars, Criteria, Trader&);
	~TripleScreen();

	void run();
private:
	struct Impl;
	std::unique_ptr<Impl> i_;
};

void runTripleScreen(chart::data::PBars bars);

}

#endif

