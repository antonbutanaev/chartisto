#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	o << "Order: enter " << result.stopEnterPrice << " stop " << result.stopPrice;
	if (result.runAway)
		o << " Run away";
	if (result.filled)
		o << " Filled " << result.filled->fillTime;
	if (result.stopped)
		o << " Stopped " << *result.stopped;
	return o;
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price stopEnterPrice,
	Price stopPrice,
	size_t orderBarNum
) {
	EntryAnalyzerParams params;
	Result result;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	Price profitDelta = 0;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {

		if (
			!result.filled && (
				(
					direction == Direction::Buy &&
					stopPrice - bars_->close(barNum) > params.runAwayFromStopK * stopDelta
				) || (
					direction == Direction::Sell &&
					bars_->close(barNum) - stopPrice > params.runAwayFromStopK * stopDelta
				)
			)
		) {
			result.runAway = true;
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
				bars_->time(barNum),
			};

		if (
			result.filled &&
			!result.stopped && (
				(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
				(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
			) && (
				bars_->time(barNum) != result.filled->fillTime ||
				barNum % params.stopOnSameDayEveryNthTime == 0
			)
		)
			result.stopped = bars_->time(barNum);

		if (result.filled) {
			const auto profit = direction == Direction::Buy?
				bars_->high(barNum) - stopEnterPrice :
				stopEnterPrice - bars_->low(barNum);

			if (profitDelta < profit) {
				profitDelta = profit;
				result.filled->profitPerStopK = profitDelta / stopDelta;
				result.filled->profitTime = bars_->time(barNum);
			}
		}

		if (result.stopped)
			break;
	}

	return result;
}

}
