#include <robotrade/trader.h>

using namespace chart;

namespace robotrade {

struct Trader::Impl {
    Impl(size_t lotSize, Price maxLoss) : lotSize(lotSize), maxLoss(maxLoss) {
    }

    void trade(int /*num*/, Price /*price*/, Price /*stop*/) {
    }

    Report report() const {
        return {};
    }

    size_t lotSize;
    Price maxLoss;
};

Trader::Trader(size_t lotSize, Price maxLoss) : i_(new Impl(lotSize, maxLoss)) {
}

Trader::~Trader() {}

void Trader::trade(int num, Price price, Price stop) {
    i_->trade(num, price, stop);

}

Trader::Report Trader::report() const {
    return i_->report();
}

}
