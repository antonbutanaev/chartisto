#include <robotrade/entryAnalyzer.h>
#include <util/hasher.h>

using namespace chart;
using namespace std;

namespace robotrade {

ostream &operator<<(ostream &o, const EntryAnalyzer::Result &result) {
	using chart::operator <<;
	o
		<< "ORDER "
		<< (result.stopEnterPrice > result.stopPrice? "Buy" : "Sell")
		<< " " << result.orderActivated
		<< " enter " << result.stopEnterPrice
		<< " target " << result.targetPrice
		<< " stop " << result.stopPrice
		<< " run away " << result.runAwayPrice[0] << '-' << result.runAwayPrice[1]
		<< ';';

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

	if (result.lossless)
		o << " Lossless on " << result.lossless->time << ';';

	if (result.stopped) {
		o << " Stopped " << result.stopped->time;
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
	std::ostream &result,
	unsigned seed
):
	params_(params),
	bars_(bars),
	probabilityProvider_(probabilityProvider),
	result_(result),
	seed_(seed)
{
}

EntryAnalyzer::Result EntryAnalyzer::analyze(
	Direction direction,
	Price stopEnterPrice,
	Price stopPrice,
	Price targetPrice,
	size_t orderBarNum
) {
	const auto buy = direction == Direction::Buy;
	const auto sell = direction == Direction::Sell;

	probabilityProvider_.seed(util::makeHash(bars_->title(0), orderBarNum, seed_));
	Result result;
	result.orderActivated = bars_->time(orderBarNum);
	result.stopEnterPrice = stopEnterPrice;
	result.stopPrice = stopPrice;
	result.targetPrice = targetPrice;
	const auto stopDelta = fabs(stopEnterPrice - stopPrice);
	const auto runAwayDelta = params_.runAwayFromStopK * stopDelta;
	result.runAwayPrice[0] = stopEnterPrice - runAwayDelta;
	result.runAwayPrice[1] = stopEnterPrice + runAwayDelta;
	const auto losslessDelta = params_.losslessStopK * stopDelta;
	for (auto barNum = orderBarNum + 1; barNum < bars_->num(); ++barNum) {
		const auto fillCondition =
			bars_->high(barNum) >= stopEnterPrice && bars_->low(barNum) <= stopEnterPrice;

		const auto wasFilled = !!result.filled;
		if (!wasFilled && fillCondition)
			result.filled = {bars_->time(barNum)};

		const auto runAwayDown =
			stopEnterPrice - bars_->close(barNum) > runAwayDelta;
		const auto runAwayUp =
			bars_->close(barNum) - stopEnterPrice > runAwayDelta;

		const auto runAwayCondition = runAwayDown || runAwayUp;

		if (!result.filled && runAwayCondition) {
			result.runAway = {
				bars_->time(barNum),
				stopEnterPrice + (runAwayDown? -runAwayDelta : runAwayDelta)
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

		const auto enterBetweenOpenAndStop =
			(buy  && bars_->open(barNum) >= stopEnterPrice) ||
			(sell && bars_->open(barNum) <= stopEnterPrice);

		const auto enterBetweenOpenAndProfit =
			(buy  && bars_->open(barNum) <= stopEnterPrice) ||
			(sell && bars_->open(barNum) >= stopEnterPrice);

		enum Probability{Probable, Certain};

		const auto runStop = [&] (Probability probability) {
			result.stopped = {
				bars_->time(barNum),
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
			if (wasFilled || stopCloseCondition || enterBetweenOpenAndStop)
				runStop(Certain);
			else if (happenedFirstOfTwo())
				runStop(Probable);
			else
				probablyNotStopped();

		} else if (targetCondition) {
			if (wasFilled || targetCloseCondition || enterBetweenOpenAndProfit)
				runProfit(Certain);
			else if (happenedFirstOfTwo())
				runProfit(Probable);
			else
				probablyNoProfit();
		}

		if (result.stopped || result.profit)
			break;

		if (!result.lossless && fabs(bars_->close(barNum) - stopEnterPrice) > losslessDelta) {
			result.lossless = {bars_->time(barNum)};
			stopPrice = stopEnterPrice;
		}
	}
	return result;
}

}
