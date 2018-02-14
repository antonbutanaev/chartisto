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

	StategyResult run() {
		LOG4CPLUS_DEBUG(logger_, "start triple screen on {} weekly and {} daily bars"_format(
			weekly_->num(), daily_->num()
		));
		StategyResult result;
		size_t weeklyPos = 0;
		size_t dailyPos = 0;

		const auto next = [&] {
			++dailyPos;
			if (weekly_->time(weeklyPos) != weekReduce(daily_->time(dailyPos)))
				++weeklyPos;
		};

		const auto end = [&] {
			if (dailyPos != daily_->num() && weeklyPos != weekly_->num())
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
			case Action::Buy:
				result.trades.emplace_back();
				break;
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

StategyResult TripleScreen::run() {
	return i_->run();
}

}
