#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	if (result.filled)
		o << "Filled (" << result.filled->time << " at " << result.filled->fillPrice << ") ";
	if (result.stopped)
		o << "Stopped (" << result.stopped->time << " at " << result.stopped->stopPrice << ") ";
	if (result.profit)
		o << "Profit (" << result.profit->time << " profitPerStop " << result.profit->profitPerStopK << ") ";
	return o;
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price stopEnterPrice,
	Price stopPrice,
	size_t orderBarNum
) {
	Result result;
	auto stopDelta = fabs(stopEnterPrice - stopPrice);
	Price profitDelta = 0;
	size_t numBars = 0;
	int stopCount = 0;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {
		++numBars;
		if (
			!result.filled && (
				(direction == Direction::Buy && bars_->high(barNum) >= stopEnterPrice) ||
				(direction == Direction::Sell && bars_->low(barNum) <= stopEnterPrice)
			)
		) {
			result.filled = {
				stopEnterPrice,
				bars_->time(barNum)
			};
		}

		if (
			result.filled &&
			!result.stopped && (
				(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
				(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
			) && (
				bars_->time(barNum) != result.filled->time ||
				++stopCount % 2 == 0
			)
		) {
			result.stopped = {
				stopPrice,
				bars_->time(barNum)
			};
		}

		const auto profit = direction == Direction::Buy?
			bars_->high(barNum) - stopEnterPrice
			:
			stopEnterPrice - bars_->low(barNum);
		if (profitDelta < profit) {
			profitDelta = profit;
			result.profit = {
				profitDelta / stopDelta,
				bars_->time(barNum)
			};
		}

		if (result.stopped)
			break;
	}

	return result;
}

}
