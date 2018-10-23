#include <util/fnv.h>
#include <robotrade/EntryAnalyzer.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	o
		<< "ORDER: " << result.orderActivated
		<< " enter " << result.stopEnterPrice
		<< " target " << result.targetPrice
		<< " stop " << result.stopPrice << ';';

	if (result.runAway)
		o << " Run away " << result.runAway->time << " " << result.runAway->price << ';';

	if (result.filled)
		o << " Filled " << result.filled->time << ';';

	if (result.profit) {
		o << " Profit " << result.profit->time;
		if (result.profit->probable)
			o << " probable";
		o << ';';
	}

	if (result.stopped) {
		o << " Stopped " << result.stopped->time;
		if (result.stopped->lossless)
			o << " lossless";
		if (result.stopped->probable)
			o << " probable";
		o << ';';
	}

	return o;
}

EntryAnalyzer::EntryAnalyzer(chart::data::PBars bars, std::ostream &result)
	:
	bars_(bars),
	result_(result)
{
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price stopEnterPrice,
	Price stopPrice,
	Price targetPrice,
	size_t orderBarNum,
	unsigned seed
) {
	const auto buy = direction == Direction::Buy;
	const auto sell = direction == Direction::Sell;

	EntryAnalyzerParams params;
	mt19937 rand;
	util::FNVHash hash;
	hash << bars_->title(0) << orderBarNum << seed;
	rand.seed(hash.value());
	uniform_real_distribution<double> dist(0, 1);
	const auto probably = [&] (double p){return dist(rand) < p;};

	Result result;
	result.orderActivated = bars_->time(orderBarNum);
	result.stopEnterPrice = stopEnterPrice;
	result.stopPrice = stopPrice;
	result.targetPrice = targetPrice;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	const auto runAwayDelta = params.runAwayFromStopK * stopDelta;
	bool losslessStop = false;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {
		const auto fillCondition =
			(buy && bars_->high(barNum) >= stopEnterPrice) ||
			(sell && bars_->low(barNum) <= stopEnterPrice);

		if (!result.filled && fillCondition)
			result.filled = {bars_->time(barNum)};

		const auto runAwayCondition =
			(buy && stopEnterPrice - bars_->close(barNum) > runAwayDelta) ||
			(sell && bars_->close(barNum) - stopEnterPrice > runAwayDelta);

		if (!result.filled && runAwayCondition) {
			result.runAway = {
				bars_->time(barNum),
				stopEnterPrice + (buy? -runAwayDelta : runAwayDelta)
			};
			break;
		}

		if (!result.filled)
			continue;

		const auto certainlyTargeted =
			(buy  && bars_->open(barNum) >= targetPrice) ||
			(sell && bars_->open(barNum) <= targetPrice);

		if (certainlyTargeted) {
			result.stopped = {
				bars_->time(barNum),
				true,
				false
			};
			break;
		}

		const auto certainlyFilled =
			bars_->time(barNum) > result.filled->time ||
			(buy  && bars_->open(barNum) >= stopEnterPrice) ||
			(sell && bars_->open(barNum) <= stopEnterPrice);

		const auto targetCondition =
			(buy  && bars_->high(barNum) >= targetPrice) ||
			(sell && bars_->low(barNum) <= targetPrice);

		const auto stopCondition =
			(buy  && bars_->low(barNum) <= stopPrice) ||
			(sell && bars_->high(barNum) >= stopPrice);

		const auto stopCloseCondition =
			(buy  && bars_->close(barNum) <= stopPrice) ||
			(sell && bars_->close(barNum) >= stopPrice);

		const auto runStop = [&] (bool probable) {
			result.stopped = {
				bars_->time(barNum),
				losslessStop,
				probable
			};
		};

		const auto runProft = [&] (bool probable) {
			result.profit = {bars_->time(barNum), probable};
		};

		if (stopCondition && targetCondition) {
			if (probably(.5))
				runStop(true);
			else
				runProft(true);
		} else if (stopCondition) {
			if (certainlyFilled || stopCloseCondition)
				runStop(false);
			else if (probably(.5))
				runStop(true);
		} else if (targetCondition)
			runProft(false);

		if (result.stopped || result.profit)
			break;

		const auto makeLosslessStop =
			(buy && bars_->close(barNum) - stopEnterPrice > stopDelta) ||
			(sell && stopEnterPrice - bars_->close(barNum) > stopDelta);

		if (makeLosslessStop) {
			stopPrice = stopEnterPrice;
			losslessStop = true;
		}
	}
	return result;
}

}
