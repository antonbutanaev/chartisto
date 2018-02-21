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
	Impl(data::PBars weekly, data::PBars daily, Criteria criteria, Trader &trader) :
		weekly_(weekly), daily_(daily), criteria_(criteria), trader_(trader) {
	}

	void run() {
		LOG4CPLUS_DEBUG(logger_, "start triple screen on {} weekly and {} daily bars"_format(
			weekly_->num(), daily_->num()
		));
		if (!weekly_->num() || !daily_->num())
			return;

		size_t tradesFound = 0;
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

				auto stopPrice = daily_->low(dailyPos);
				if (dailyPos > 0 && stopPrice > daily_->low(dailyPos - 1))
					stopPrice = daily_->low(dailyPos - 1);

				++tradesFound;
				trader_.trade({
					Trader::Trade::ByStop,
					daily_->time(dailyPos + 1),
					daily_->high(dailyPos),
					stopPrice
				});
				break;
			}
			case Action::Sell: {
				if (
					dailyPos == daily_->num() - 1 ||
					daily_->low(dailyPos + 1) >= daily_->low(dailyPos) ||
					daily_->high(dailyPos + 1) < daily_->low(dailyPos)
				)
					break;

				auto stopPrice = daily_->high(dailyPos);
				if (dailyPos > 0 && stopPrice < daily_->high(dailyPos - 1))
					stopPrice = daily_->high(dailyPos - 1);

				++tradesFound;
				trader_.trade({
					Trader::Trade::ByStop,
					daily_->time(dailyPos + 1),
					daily_->low(dailyPos),
					stopPrice
				});
				break;
			}
			case Action::Wait:
				trader_.priceChange(daily_->time(dailyPos), daily_->low(dailyPos));
				trader_.priceChange(daily_->time(dailyPos), daily_->high(dailyPos));
				break;
			}
		}
		LOG4CPLUS_DEBUG(logger_, "triple screen found {} trades"_format(tradesFound));
	}

	data::PBars weekly_;
	data::PBars daily_;
	Criteria criteria_;
	Trader &trader_;
	log4cplus::Logger logger_ = log4cplus::Logger::getInstance("TripleScreen");
};

TripleScreen::TripleScreen(data::PBars weekly, data::PBars daily, Criteria criteria, Trader &trader)
: i_(new Impl(weekly, daily, criteria, trader)) {
}

TripleScreen::~TripleScreen() {}

void TripleScreen::run() {i_->run();}

}
