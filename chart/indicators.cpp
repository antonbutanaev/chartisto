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
	) : points_(move(points)) {
		title_ = points_->title(0) + " EMA " + to_string(period);
		ema_.reserve(num());
		size_t i = 0;
		for(; points_->close(i) == NoPrice; ++i)
			ema_.push_back(NoPrice);

		ema_.push_back(points_->close(i++));

		const auto k = 2. / (period + 1);
		for (; i < num(); ++i) {
			if (points_->close(i) == NoPrice)
				continue;
			ema_.push_back(k * points_->close(i) + (1 - k) * ema_[i - 1]);
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
	data::PPoints result = make_shared<EMA>(move(points), period);
	return result;
}

data::PPoints forceIndex(data::PBars bars, size_t period) {
    auto forceIndex = data::createPoints(
        bars,
        [bars] (size_t n) {
            return n == 0 ?
                NoPrice : (bars->close(n) - bars->close(n - 1)) * bars->volume(n);
        }
    );
    return ema(move(forceIndex), period);
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
