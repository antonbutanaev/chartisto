#ifndef TRADER_H
#define TRADER_H

#include <memory>
#include <vector>
#include <chart/data.h>

namespace robotrade {

class Trader {
public:
    Trader(size_t lotSize, chart::Price maxLoss);
    ~Trader();

    void trade(int num, chart::Price price, chart::Price stop);

    struct Report {
        struct Line {};
        std::vector<Line> lines;
    };

    Report report() const;
private:
    struct Impl;
    std::unique_ptr<Impl> i_;
};

}

#endif
