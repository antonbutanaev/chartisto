#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	if (result.filled)
		o << "Filled " << result.filled->time << " price " << result.filled->fillPrice << " ";
	if (result.stopped)
		o << "Stopped " << result.stopped->time << " price " << result.stopped->stopPrice << " ";
	if (result.profit)
		o << "Profit " << result.profit->time << " profitPerStop " << result.profit->profitPerStopK << " ";
	return o;
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price limitPrice,
	Price stopPrice,
	size_t orderBarNum
) {
	Result result;
	auto stopDelta = fabs(limitPrice - stopPrice);
	Price profitDelta = 0;
	size_t numBars = 0;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {
		++numBars;
		if (
			!result.filled && (
				(direction == Direction::Buy && bars_->low(barNum) <= limitPrice) ||
				(direction == Direction::Sell && bars_->high(barNum) >= limitPrice)
			)
		) {
			result.filled = {
				limitPrice,
				bars_->time(barNum)
			};
		}

		if (
			!result.stopped && (
				(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
				(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
			)
		) {
			result.stopped = {
				stopPrice,
				bars_->time(barNum)
			};
		}

		if (result.stopped)
			break;

		const auto profit = direction == Direction::Buy?
			bars_->high(barNum) - limitPrice
			:
			limitPrice - bars_->low(barNum);
		if (profitDelta < profit) {
			profitDelta = profit;
			result.profit = {
				profitDelta / stopDelta,
				bars_->time(barNum)
			};
		}
	}

	return result;
}

}
