#include <string>
#include <vector>
#include <chart/ema.h>

using namespace std;
using namespace chart;

namespace {

class EMA: public data::Points {
public:
	EMA(
		const data::PPoints &points,
		size_t period
	) : points_(points) {
		title_ = points_->title() + " EMA " + to_string(period);
		ema_.reserve(num());
		ema_.push_back(points_->close(0));
		const auto k = 2. / (period + 1);
		for (size_t i = 1; i < num(); ++i)
			ema_.push_back(
				k * points_->close(i) + (1 - k) * ema_[i - 1]
			);
	}

    std::string title() const override {return title_;}
    Time time(size_t n) const override {return points_->time(n);}
    Price close(size_t n) const override {return ema_[n];}
    size_t num() const override {return points_->num();}

private:
    data::PPoints points_;
    string title_;
    vector<Price> ema_;
};

};

namespace chart {

data::PPoints ema(const data::PPoints &points, size_t period) {
	data::PPoints result = make_shared<EMA>(points, period);
	return result;
}

}
