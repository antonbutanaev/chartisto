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
		size_t weeklyPos = 0;
		size_t dailyPos = 0;
		const auto weeklyTime = weekly_->time(weeklyPos);
		const auto dailyTime = daily_->time(dailyPos);

		if (weeklyTime != weekReduce(weeklyTime))
			throw runtime_error("Bad weekly time {}:{} at pos"_format(weeklyTime, weeklyPos));
		if (weeklyTime != weekReduce(dailyTime))
			throw runtime_error("Daily {}:{} and weekly {}:{} time not in sync"_format(
				dailyTime, dailyPos, weeklyTime, weeklyPos
			));

		return {};
	}

	data::PBars weekly_;
	data::PBars daily_;
	Criteria criteria_;
};

TripleScreen::TripleScreen(data::PBars weekly, data::PBars daily, Criteria criteria)
:
i_(new Impl(weekly, daily, criteria)) {
}

TripleScreen::~TripleScreen() {}

StategyResult TripleScreen::run() {
	return i_->run();
}

}
