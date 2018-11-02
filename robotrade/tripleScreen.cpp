#include <iostream>
#include <log4cplus/loggingmacros.h>
#include <robotrade/tripleScreen.h>
#include <chart/reduce.h>
#include <chart/indicators.h>
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
					trader_.position() > 0 ||
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
					Trader::Trade::CloseSell,
					daily_->time(dailyPos),
					daily_->close(dailyPos),
					NoPrice
				});
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
					trader_.position() < 0 ||
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
					Trader::Trade::CloseBuy,
					daily_->time(dailyPos),
					daily_->close(dailyPos),
					NoPrice
				});
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

using chart::indicators::macd;
using robotrade::TripleScreen;

using namespace std;
using namespace chart;
using namespace robotrade;

void runTripleScreen(data::PBars bars) {
	const auto barsDaily = reduce(*bars, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	const auto closeWeekly = data::createPoints(barsWeekly, [&](size_t n) {return barsWeekly->close(n);});
	const auto macdWeekly = indicators::macd(closeWeekly, 13, 26, 7);

	const auto forceIndex = indicators::forceIndex(barsDaily, 2);

	const auto optional = [](auto value, auto no, auto sep) {
		if (value == no)
			cout << sep;
		else
			cout << value << sep;
	};

	Trader trader({
		100, 2000,
		[&](const Trader::OnTrade &trade) {
			cout
				<< trade.time << ','
				<< trade.num << ','
				<< trade.price << ',';
			optional(trade.gain, NoPrice, ',');
			cout
				<< trade.total << ','
				<< trade.pos << '\n';

		}
	});

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			if (weekly == 0)
			return Action::Wait;
			if (
				macdWeekly->histogram->close(weekly-1) < macdWeekly->histogram->close(weekly) &&
				macdWeekly->histogram->close(weekly) < 0 &&
				forceIndex->close(daily) < 0
			)
				return Action::Buy;
			else if (
				macdWeekly->histogram->close(weekly-1) > macdWeekly->histogram->close(weekly) &&
				macdWeekly->histogram->close(weekly) > 0 &&
				forceIndex->close(daily) > 0
			)
				return Action::Sell;
			else
				return Action::Wait;
		},
		trader
	);

	cout << "time,num,price,gain,total,position\n";
	tripleScreen.run();
	trader.trade({Trader::Trade::Close, barsDaily->time(barsDaily->num()-1), barsDaily->close(barsDaily->num()-1), NoPrice});
}

}
