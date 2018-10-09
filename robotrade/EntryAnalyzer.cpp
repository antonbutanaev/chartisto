#include <robotrade/EntryAnalyzer.h>

namespace robotrade {

std::optional<EntryAnalyzer::Result> EntryAnalyzer::analyze(
	Direction direction,
	chart::Price limitOrderPrice,
	chart::Price stopPrice,
	size_t orderBarNum
) {
	bool filled = false;
	for (auto barNum = orderBarNum; barNum < bars_->num(); ++barNum) {
		filled = filled || (
			(direction == Direction::Buy && bars_->low(barNum) <= limitOrderPrice) ||
			(direction == Direction::Sell && bars_->high(barNum) >= limitOrderPrice)
		);
		bool stopped = (
			(direction == Direction::Buy && bars_->low(barNum) <= stopPrice) ||
			(direction == Direction::Sell && bars_->high(barNum) >= stopPrice)
		);

		if (stopped) {
			break;
		}

	}
	return {};
}

}
