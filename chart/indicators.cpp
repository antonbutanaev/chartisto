#include <string>
#include <vector>
#include <chart/indicators.h>

using namespace std;

namespace chart {
namespace indicators {
namespace {

class EMA: public data::Points {
public:
	EMA(
		data::PPoints &&points,
		size_t period
	) : points_(move(points))
	{
		title_ = points_->title(0) + " EMA " + to_string(period);
		ema_.reserve(num());
		size_t barNum = 0;
		for(; points_->close(barNum) == NoPrice; ++barNum)
			ema_.push_back(NoPrice);

		ema_.push_back(points_->close(barNum++));
		auto prevClose = ema_.back();

		const auto k = 2. / (period + 1);
		for (; barNum < num(); ++barNum) {
			auto close = points_->close(barNum);
			if (close == NoPrice)
				close = prevClose;

			ema_.push_back(k * close + (1 - k) * ema_.back());
			prevClose = close;
		}
	}

    std::string title(size_t) const override {return title_;}
    Time time(size_t n) const override {return points_->time(n);}
    Price close(size_t n) const override {return ema_[n];}
    size_t num() const override {return points_->num();}

private:
    data::PPoints points_;
    string title_;
    vector<Price> ema_;
};

};

data::PPoints ema(data::PPoints points, size_t period) {
	return make_shared<EMA>(move(points), period);
}

data::PPoints forceIndex(data::PBars bars, size_t period) {
    return ema(
		data::createPoints(
			bars,
			[bars] (size_t barNum) {
				return barNum == 0 ?
					NoPrice : (bars->close(barNum) - bars->close(barNum - 1)) * bars->volume(barNum);
			}
		),
		period
    );
}

data::PPoints atr(data::PBars bars, size_t period) {
	return ema(
		data::createPoints(
			bars,
			[bars](size_t barNum){
				if (barNum == 0)
					return bars->high(0) - bars->low(0);
				else {
					const auto prevClose = bars->close(barNum - 1);
					return max({
						bars->high(barNum) - bars->low(barNum),
						fabs(bars->high(barNum) - prevClose),
						fabs(bars->low(barNum) - prevClose)
					});
				}
			}
		),
		period
	);
}

shared_ptr<Macd> macd(data::PPoints points, size_t fastPeriod, size_t slowPeriod, size_t signalPeriod) {
    auto result = make_shared<Macd>();
    result->fastEma = ema(points, fastPeriod);
    result->slowEma = ema(points, slowPeriod);
    result->macd = data::createPoints(
        result->fastEma,
        [fastEma = result->fastEma, slowEma = result->slowEma] (size_t n) {
            return fastEma->close(n) - slowEma->close(n);
        }
    );
    result->signal = ema(result->macd, signalPeriod);
    result->histogram = data::createPoints(
        result->fastEma,
        [macd = result->macd, signal = result->signal] (size_t n) {
            return macd->close(n) - signal->close(n);
        }
    );
    return result;
}

}}
