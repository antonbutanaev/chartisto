#ifndef CHART_MACD_H
#define CHART_MACD_H

#include <memory>
#include <chart/data.h>

namespace chart {

struct Macd {
    data::PPoints slowEma;
    data::PPoints fastEma;
    data::PPoints macd;
    data::PPoints signal;
    data::PPoints histogram;
};

std::shared_ptr<Macd> macd(data::PPoints, size_t fastPeriod, size_t slowPeriod, size_t signalPeriod);

}

#endif
