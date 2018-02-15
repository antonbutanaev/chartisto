#include <log4cplus/loggingmacros.h>
#include <robotrade/triplescreen.h>
#include <chart/reduce.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <date/date.h>

using namespace chart;
using namespace std;
using namespace fmt::literals;
using namespace date;

namespace robotrade {

struct TripleScreen::Impl {
	Impl(data::PBars weekly, data::PBars daily, Criteria criteria) :
		weekly_(weekly), daily_(daily), criteria_(criteria) {
	}

	StrategyResult run() {
		LOG4CPLUS_DEBUG(logger_, "start triple screen on {} weekly and {} daily bars"_format(
			weekly_->num(), daily_->num()
		));
		StrategyResult result;
		if (!weekly_->num() || !daily_->num())
			return result;

		size_t weeklyPos = 0;
		size_t dailyPos = 0;

		const auto next = [&] {
			++dailyPos;
			if (weekly_->time(weeklyPos) != weekReduce(daily_->time(dailyPos)))
				++weeklyPos;
		};

		const auto end = [&] {
			if (dailyPos == daily_->num() || weeklyPos == weekly_->num())
				return true;

			const auto weeklyTime = weekly_->time(weeklyPos);
			const auto dailyTime = daily_->time(dailyPos);

			if (dailyTime != dayReduce(dailyTime))
				throw runtime_error("Bad daily time {},{}"_format(dailyTime, dailyPos));
			if (weeklyTime != weekReduce(weeklyTime))
				throw runtime_error("Bad weekly time {},{}"_format(weeklyTime, weeklyPos));
			if (weeklyTime != weekReduce(dailyTime))
				throw runtime_error("Daily {},{} and weekly {},{} time not in sync"_format(
					dailyTime, dailyPos, weeklyTime, weeklyPos
				));

			return false;
		};

		for (; !end(); next()) {
			switch (criteria_(weeklyPos, dailyPos)) {
			case Action::Buy: {
				if (
					dailyPos == daily_->num() - 1 ||
					daily_->high(dailyPos + 1) <= daily_->high(dailyPos) ||
					daily_->low(dailyPos + 1) > daily_->high(dailyPos)
				)
					break;

				result.trades.emplace_back();
				auto &trade = result.trades.back();
				trade.time = daily_->time(dailyPos + 1);
				trade.number = 1;
				trade.enterPrice = daily_->high(dailyPos + 1);
				trade.stopPrice = daily_->low(dailyPos);
				if (dailyPos > 0 && trade.stopPrice > daily_->low(dailyPos - 1))
					trade.stopPrice = daily_->low(dailyPos - 1);

				for (auto i = dailyPos + 1; i < daily_->num(); ++i) {
					if (
						trade.maxProfitToStop == NoPrice ||
						trade.maxProfitToStop < daily_->high(i) - trade.enterPrice
					)
						trade.maxProfitToStop = daily_->high(i) - trade.enterPrice;

					if (daily_->low(i) <= trade.stopPrice) {
						trade.barsToStop = i - (dailyPos + 1);
						trade.stopTime = daily_->time(i);
						break;
					}
				}
				break;
			}
			case Action::Sell:
				result.trades.emplace_back();
				break;
			case Action::Wait:
				break;
			}
		}
		return result;
	}

	data::PBars weekly_;
	data::PBars daily_;
	Criteria criteria_;
	log4cplus::Logger logger_ = log4cplus::Logger::getInstance("TripleScreen");
};

TripleScreen::TripleScreen(data::PBars weekly, data::PBars daily, Criteria criteria)
: i_(new Impl(weekly, daily, criteria)) {
}

TripleScreen::~TripleScreen() {}

StrategyResult TripleScreen::run() {
	return i_->run();
}

}
