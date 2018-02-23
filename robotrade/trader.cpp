#include <list>
#include <robotrade/trader.h>

using namespace chart;
using namespace std;

namespace robotrade {

struct Trader::Impl {
    Impl(const Params &params) : params_(params) {}

    void trade(const Trade &trade) {
        int num  = 0;
        switch (trade.num) {
        case Trade::ByStop:
            num = static_cast<int>(params_.maxLoss / (trade.price - trade.stopPrice) / params_.lotSize);
            num *= params_.lotSize;
            break;

        case Trade::Close:
            eraseStops([](const auto&) {return true;});
            num = -position_;
            break;

        case Trade::CloseBuy:
            if (position_ > 0) {
                eraseStops([](const auto &stop) {return stop.num < 0;});
                num = -position_;
            } else
                num = 0;
            break;

        case Trade::CloseSell:
            if (position_ < 0) {
                eraseStops([](const auto &stop) {return stop.num > 0;});
                num = -position_;
            } else
                num = 0;
            break;

        default:
            num = trade.num;
            num /= params_.lotSize;
            num *= params_.lotSize;
            break;
        }

        if (!num)
            return;

        const auto avgCost = position_? cost_ / position_ : 0.;
        const auto prevPosition = position_;
        position_ += num;

        const auto sign = [] (auto x) {return (x > 0) - (x < 0);};

        auto gain = NoPrice;
        if (prevPosition && position_ && sign(prevPosition) != sign(position_)) {
            gain = prevPosition * (trade.price - avgCost);
            cost_ = position_ * trade.price;
        } else {
            if (abs(prevPosition) > abs(position_)) {
                const auto delta = prevPosition - position_;
                gain = delta * (trade.price - avgCost);
                cost_ += gain;
            }
            cost_ += num * trade.price;
        }

        const auto sum = num * trade.price;
        total_ -= sum;

        if (trade.stopPrice != NoPrice)
            stops_.push_back({trade.stopPrice, -num});

        params_.onTrade({
            trade.time,
            num,
            trade.price,
            gain,
            total_,
            position_
        });

        if (!position_) {
            cost_ = 0;
        }
    }

    void priceChange(Time time, Price price) {
        eraseStops([&](const auto &stop){
            if (
                (stop.num < 0 && price <= stop.price) ||
                (stop.num > 0 && price >= stop.price)
            ) {
                trade({stop.num, time, stop.price, NoPrice});
                return true;
            } else
                return false;
        });
    }

    template<class F> void eraseStops(const F &func) {
        for (auto it = stops_.begin(); it != stops_.end(); )
            if (func(*it))
                it = stops_.erase(it);
            else
                ++it;
  }

    int position_ = 0;
    Price total_ = 0.;
    Price cost_ = 0.;

    struct Stop {
        Price price;
        int num;
    };

    list<Stop> stops_;
    Params params_;
};

Trader::Trader(const Params &params) : i_(new Impl(params)) {
}

Trader::~Trader() {}

void Trader::trade(const Trade &trade) {
    i_->trade(trade);

}

void Trader::priceChange(Time time, Price price) {
    i_->priceChange(time, price);
}

int Trader::position() const {
    return i_->position_;
}

}
