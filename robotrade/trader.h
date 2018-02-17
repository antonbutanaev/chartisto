#ifndef TRADER_H
#define TRADER_H

#include <memory>
#include <functional>
#include <chart/data.h>

namespace robotrade {

class Trader {
public:
    struct Params {
        size_t lotSize;
        chart::Price maxLoss;
        std::function<void(
            chart::Time time,
            int num,
            chart::Price price,
            chart::Price gain,
            chart::Price totalGain
        )> onTrade;
    };

    Trader(const Params &params);
    ~Trader();

    struct Trade {
        chart::Time time;
        int num;
        chart::Price price;
        chart::Price stopPrice;
    };
    void trade(const Trade&);

    void priceChange(chart::Time time, chart::Price price);

private:
    struct Impl;
    std::unique_ptr<Impl> i_;
};

}

#endif
