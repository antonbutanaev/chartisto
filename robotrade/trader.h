#ifndef TRADER_H
#define TRADER_H

#include <memory>
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
        size_t lotSize;
        chart::Price maxLoss;
        std::function<void(const OnTrade&)> onTrade;
    };

    struct Trade {
        enum class Type {ByStop, Close, UseNum};
        Type type;
        int num;
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
