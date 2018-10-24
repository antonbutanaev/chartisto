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

void ProbabilityProvider::seed(unsigned value) {
	rand_.seed(value);
}

bool ProbabilityProvider::happened(double probability) {
	return dist_(rand_) < probability;
}

EntryAnalyzer::EntryAnalyzer(
	chart::data::PBars bars,
	IProbabilityProvider &probabilityProvider,
	std::ostream &result
)
	:
	bars_(bars),
	probabilityProvider_(probabilityProvider),
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
	util::FNVHash hash;
	hash << bars_->title(0) << orderBarNum << seed;
	probabilityProvider_.seed(hash.value());
	const auto probablyHappened = [&] (double probability){
		return probabilityProvider_.happened(probability);
	};

	Result result;
	result.orderActivated = bars_->time(orderBarNum);
	result.stopEnterPrice = stopEnterPrice;
	result.stopPrice = stopPrice;
	result.targetPrice = targetPrice;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	const auto runAwayDelta = params.runAwayFromStopK * stopDelta;
	bool stopMadeLossless = false;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {
		const auto fillCondition =
			bars_->high(barNum) >= stopEnterPrice && bars_->low(barNum) <= stopEnterPrice;

		const auto wasFilled = !!result.filled;
		if (!wasFilled && fillCondition)
			result.filled = {bars_->time(barNum)};

		const auto runAwayCondition =
			fabs(stopEnterPrice - bars_->close(barNum)) > runAwayDelta;

		if (!result.filled && runAwayCondition) {
			result.runAway = {
				bars_->time(barNum),
				stopEnterPrice + (buy? -runAwayDelta : runAwayDelta)
			};
			break;
		}

		if (!result.filled)
			break;

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
				fabs(stopPrice - stopEnterPrice) < PriceEpsilon,
				probable
			};
		};

		const auto runProft = [&] (bool probable) {
			result.profit = {bars_->time(barNum), probable};
		};

		if (stopCondition && targetCondition) {
			if (probablyHappened(.5))
				runStop(true);
			else
				runProft(true);
		} else if (stopCondition) {
			if (wasFilled || stopCloseCondition)
				runStop(false);
			else if (probablyHappened(.5))
				runStop(true);
		} else if (targetCondition)
			runProft(false);

		if (result.stopped || result.profit)
			break;

		if (!stopMadeLossless && fabs(bars_->close(barNum) - stopEnterPrice) > stopDelta) {
			stopMadeLossless = true;
			stopPrice = stopEnterPrice;
		}
	}
	return result;
}

}
