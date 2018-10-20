#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	o
		<< "Order: " << result.orderActivated
		<< " enter " << result.stopEnterPrice
		<< " stop " << result.stopPrice << ';';
	if (result.runAway)
		o << " Run away " << result.runAway->time << " " << result.runAway->price << ';';
	if (result.filled) {
		o
			<< " Filled " << result.filled->fillTime
			<< " profit " << result.filled->profitPerStopK
			<< " on " << result.filled->profitTime << ';';

	}

	if (result.onStopDayProfit)
		o << " profit on stop day " << *result.onStopDayProfit << ';';

	if (result.stopped) {
		o << " Stopped " << result.stopped->time << ';';
	}

	if (result.onFillDayStop)
		o << " stop on fill day " << *result.onFillDayStop << ';';

	return o;
}

EntryAnalyzer::EntryAnalyzer(chart::data::PBars bars) : bars_(bars) {
	rand_.seed(hash<string>()(bars->title(0)));
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price stopEnterPrice,
	Price stopPrice,
	size_t orderBarNum
) {
	EntryAnalyzerParams params;
	const auto probablyHappens = [&]{return rand_() % params.stopOnSameDayEveryNthTime == 0;};
	Result result;
	result.orderActivated = bars_->time(orderBarNum);
	result.stopEnterPrice = stopEnterPrice;
	result.stopPrice = stopPrice;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	Price profitDelta = 0;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {

		if (
			!result.filled && (
				(
					direction == Direction::Buy &&
					stopEnterPrice - bars_->close(barNum) > params.runAwayFromStopK * stopDelta
				) || (
					direction == Direction::Sell &&
					bars_->close(barNum) - stopEnterPrice > params.runAwayFromStopK * stopDelta
				)
			)
		) {
			result.runAway = {
				bars_->time(barNum),
				(direction == Direction::Buy? -1:1) * params.runAwayFromStopK * stopDelta + stopEnterPrice
			};
			break;
		}

		if (
			!result.filled && (
				(
					direction == Direction::Buy &&
					bars_->high(barNum) >= stopEnterPrice &&
					bars_->low(barNum) <= stopEnterPrice
				) || (
					direction == Direction::Sell &&
					bars_->low(barNum) <= stopEnterPrice &&
					bars_->high(barNum) >= stopEnterPrice
				)
			)
		)
			result.filled = {
				bars_->time(barNum),
				0,
				bars_->time(barNum)
			};

		if (
			result.filled &&
			!result.stopped && (
				(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
				(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
			)
		) {
			const auto runStop = [&] {result.stopped = {bars_->time(barNum)};};
			if (bars_->time(barNum) != result.filled->fillTime)
				runStop();
			else {
				result.onFillDayStop = probablyHappens();
				if (*result.onFillDayStop)
					runStop();
			}
		}

		if (result.filled) {
			const auto calcProfit = [&] {
				const auto profit = direction == Direction::Buy?
					bars_->high(barNum) - stopEnterPrice :
					stopEnterPrice - bars_->low(barNum);

				if (profitDelta < profit) {
					profitDelta = profit;
					result.filled->profitPerStopK = profitDelta / stopDelta;
					result.filled->profitTime = bars_->time(barNum);
				}
			};

			if (!result.stopped)
				calcProfit();
			else {
				result.onStopDayProfit = probablyHappens();
				if (*result.onStopDayProfit)
					calcProfit();
			}
		}

		if (result.stopped)
			break;
	}

	return result;
}

}
