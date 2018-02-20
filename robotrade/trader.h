#ifndef TRADER_H
#define TRADER_H

#include <memory>
#include <limits>
#include <functional>
#include <chart/data.h>

namespace robotrade {

class Trader {
public:
    struct OnTrade {
        chart::Time time;
        int num;
        chart::Price price, gain, total;
    };

    struct Params {
        int lotSize;
        chart::Price maxLoss;
        std::function<void(const OnTrade&)> onTrade;
    };

    struct Trade {
        int num;
        static constexpr auto ByStop = std::numeric_limits<decltype(num)>::max();
        static constexpr auto Close = ByStop - 1;
        static constexpr auto CloseSell = ByStop - 2;
        static constexpr auto CloseBuy = ByStop - 3;
        chart::Time time;
        chart::Price price, stopPrice;
    };

    Trader(const Params &params);
    ~Trader();

    void trade(const Trade&);
    void priceChange(chart::Time time, chart::Price price);

private:
    struct Impl;
    std::unique_ptr<Impl> i_;
};

}

#endif
