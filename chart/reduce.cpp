#include <iostream>
#include <vector>
#include <chart/reduce.h>
#include <date/date.h>

using namespace std;

namespace chart {
namespace {

class Bars: public data::Bars {
public:
    Bars(const data::Bars &bars, const ReduceFunc &reduceFunc) {
        title_ = bars.title();
        for (size_t i = 0; i < bars.num(); ++i) {
            const auto time = reduceFunc(bars.time(i));

            if (bars_.empty() || bars_.back().time != time)
                bars_.push_back({
                    time, bars.open(i), bars.close(i), bars.high(i), bars.low(i), bars.volume(i)
                });
            else {
                bars_.back().close = bars.close(i);
                bars_.back().volume += bars.volume(i);

                if (bars_.back().high < bars.high(i))
                    bars_.back().high = bars.high(i);
                if (bars_.back().low > bars.low(i))
                    bars_.back().low = bars.low(i);
            }
        }
    }

private:
    std::string title() const override {return title_;}
    Time time(size_t n) const override {return bars_[n].time;}
    Price open(size_t n) const override {return bars_[n].open;}
    Price close(size_t n) const override {return bars_[n].close;}
    Price high(size_t n) const override {return bars_[n].high;}
    Price low(size_t n) const override {return bars_[n].low;}
    Volume volume(size_t n) const override {return bars_[n].volume;}

    size_t num() const override {return bars_.size();}

    struct Bar {
        Time time;
        Price open, close, high, low;
        Volume volume;
    };
    vector<Bar> bars_;
    string title_;
};

}

data::PBars reduce(const data::Bars &bars, const ReduceFunc &reduceFunc) {
    return make_shared<Bars>(bars, reduceFunc);
}

Time weekReduce(Time time) {
    using namespace date;
    using namespace std::chrono;

    sys_days sd = time_point_cast<days>(time);
    return sd - (weekday{sd} - mon);

}

}
