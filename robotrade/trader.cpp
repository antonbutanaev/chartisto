#include <vector>
#include <robotrade/trader.h>

using namespace chart;
using namespace std;

namespace robotrade {

struct Trader::Impl {
    Impl(const Params &params) : params(params) {}

    void trade(const Trade &trade) {
        position += trade.num;
        total -= trade.num * trade.price;

        stops.push_back({trade.stopPrice, trade.num});

        params.onTrade(
            trade.time,
            trade.num,
            trade.price,
            0,
            0
        );
    }

    void priceChange(Time, Price) {

    }

    int position = 0;
    Price total = 0.;

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
