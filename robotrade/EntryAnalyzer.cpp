#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

std::optional<EntryAnalyzer::Result> EntryAnalyzer::analyze(
	Direction direction,
	Price limitOrderPrice,
	Price stopPrice,
	size_t orderBarNum
) {
	bool filled = false;
	bool stopped = false;
	auto stopDelta = fabs(limitOrderPrice - stopPrice);
	Price profitDelta = 0;
	size_t numBars = 0;
	for (auto barNum = orderBarNum; barNum < bars_->num(); ++barNum) {
		++numBars;
		filled = filled || (
			(direction == Direction::Buy && bars_->low(barNum) <= limitOrderPrice) ||
			(direction == Direction::Sell && bars_->high(barNum) >= limitOrderPrice)
		);
		stopped = (
			(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
			(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
		);

		if (stopped) {
			break;
		}

		if (filled)
			profitDelta = max(
				profitDelta,
				direction == Direction::Buy?
					bars_->high(barNum) - limitOrderPrice :
					limitOrderPrice - bars_->low(barNum)
			);

	}

	if (!filled)
		return {};

	Result result{numBars, {}};
	if (!stopped)
		result.profitK = profitDelta / stopDelta;

	return result;
}

}
