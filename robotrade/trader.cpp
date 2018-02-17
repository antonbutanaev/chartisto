#include <vector>
#include <robotrade/trader.h>

using namespace chart;
using namespace std;

namespace robotrade {

struct Trader::Impl {
    Impl(const Params &params) : params(params) {}

    void trade(const Trade &trade) {
        const auto prevPosition = position;
        const auto avgPositionCost = position? cost / position : 0.;

        position += trade.num;
        if (!position)
            cost = 0;

        auto gain = NoPrice;
        if (abs(prevPosition) > abs(position))
            gain = (position - prevPosition) * (trade.price - avgPositionCost);

        const auto sum = trade.num * trade.price;
        total -= sum;
        cost += sum;

        stops.push_back({trade.stopPrice, trade.num});

        params.onTrade(
            trade.time,
            trade.num,
            trade.price,
            gain,
            total
        );
    }

    void priceChange(Time, Price) {
    }

    int position = 0;
    Price total = 0.;
    Price cost = 0.;

    struct Stop {
        Price price;
        int num;
    };

    vector<Stop> stops;

    Params params;
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
