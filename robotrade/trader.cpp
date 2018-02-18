#include <vector>
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
            num = -position_;
            break;
        default:
            num = trade.num;
            num /= params_.lotSize;
            num *= params_.lotSize;
            break;
        }

        const auto prevPosition = position_;
        const auto avgCost = position_? cost_ / position_ : 0.;

        position_ += num;

        const auto sign = [] (auto x) {return (x > 0) - (x < 0);};

        auto gain = NoPrice;
        if (prevPosition != 0 && sign(prevPosition) != sign(position_))
            gain = (prevPosition) * (trade.price - avgCost);
        else if (abs(prevPosition) > abs(position_))
            gain = (prevPosition - position_) * (trade.price - avgCost);

        if (gain != NoPrice)
            cost_ = 0;

        const auto sum = num * trade.price;
        total_ -= sum;
        cost_ += sum;

        stops_.push_back({trade.stopPrice, num});

        params_.onTrade({
            trade.time,
            num,
            trade.price,
            gain,
            total_
        });
    }

    void priceChange(Time, Price) {
    }

    int position_ = 0;
    Price total_ = 0.;
    Price cost_ = 0.;

    struct Stop {
        Price price;
        int num;
    };

    vector<Stop> stops_;
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

}
