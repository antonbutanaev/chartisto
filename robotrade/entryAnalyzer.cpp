#include <util/hash.h>
#include <robotrade/entryAnalyzer.h>

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
			o << " probably";
		o << ';';
	}

	if (!result.probablyNoProfit.empty()) {
		o << " probably no profit on";
		for (const auto &time: result.probablyNoProfit)
			o << ' ' << time;
		o << ';';
	}

	if (result.stopped) {
		o << " Stopped " << result.stopped->time;
		if (result.stopped->lossless)
			o << " lossless";
		if (result.stopped->probable)
			o << " probably";
		o << ';';
	}

	if (!result.probablyNotStopped.empty()) {
		o << " probably not stopped on";
		for (const auto &time: result.probablyNotStopped)
			o << ' ' << time;
		o << ';';
	}

	return o;
}

void ProbabilityProvider::seed(unsigned value) {
	rand_.seed(value);
}

unsigned ProbabilityProvider::whatHappened(unsigned numChances) {
	return 1 + static_cast<unsigned>(dist_(rand_) / (1. / numChances));
}

EntryAnalyzer::EntryAnalyzer(
	const EntryAnalyzerParams &params,
	chart::data::PBars bars,
	IProbabilityProvider &probabilityProvider,
	std::ostream &result
)
	:
	params_(params),
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

	util::hash::FNV1a hash;
	hash << bars_->title(0) << orderBarNum << seed;
	probabilityProvider_.seed(hash);
	Result result;
	result.orderActivated = bars_->time(orderBarNum);
	result.stopEnterPrice = stopEnterPrice;
	result.stopPrice = stopPrice;
	result.targetPrice = targetPrice;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	const auto runAwayDelta = params_.runAwayFromStopK * stopDelta;
	const auto losslessDelta = params_.losslessStopK * stopDelta;
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
			continue;

		const auto targetCondition =
			(buy  && bars_->high(barNum) >= targetPrice) ||
			(sell && bars_->low(barNum) <= targetPrice);

		const auto targetCloseCondition =
			(buy  && bars_->close(barNum) >= targetPrice) ||
			(sell && bars_->close(barNum) <= targetPrice);


		const auto stopCondition =
			(buy  && bars_->low(barNum) <= stopPrice) ||
			(sell && bars_->high(barNum) >= stopPrice);

		const auto stopCloseCondition =
			(buy  && bars_->close(barNum) <= stopPrice) ||
			(sell && bars_->close(barNum) >= stopPrice);

		enum Probability{Probable, Certain};

		const auto runStop = [&] (Probability probability) {
			result.stopped = {
				bars_->time(barNum),
				fabs(stopPrice - stopEnterPrice) < PriceEpsilon,
				probability == Probable
			};
		};

		const auto probablyNotStopped = [&]{
			result.probablyNotStopped.push_back(bars_->time(barNum));
		};

		const auto runProfit = [&] (Probability probability) {
			result.profit = {bars_->time(barNum), probability == Probable};
		};

		const auto probablyNoProfit = [&]{
			result.probablyNoProfit.push_back(bars_->time(barNum));
		};

		const auto happenedFirstOfTwo = [&]{
			return 1 == probabilityProvider_.whatHappened(2);
		};

		if (stopCondition && targetCondition) {
			if (happenedFirstOfTwo()) {
				runStop(Probable);
				probablyNoProfit();
			} else {
				runProfit(Probable);
				probablyNotStopped();
			}

		} else if (stopCondition) {
			if (wasFilled || stopCloseCondition)
				runStop(Certain);
			else if (happenedFirstOfTwo())
				runStop(Probable);
			else
				probablyNotStopped();

		} else if (targetCondition) {
			if (wasFilled || targetCloseCondition)
				runProfit(Certain);
			else if (happenedFirstOfTwo())
				runProfit(Probable);
			else
				probablyNoProfit();
		}

		if (result.stopped || result.profit)
			break;

		if (!stopMadeLossless && fabs(bars_->close(barNum) - stopEnterPrice) > losslessDelta) {
			stopMadeLossless = true;
			stopPrice = stopEnterPrice;
		}
	}
	return result;
}

}
