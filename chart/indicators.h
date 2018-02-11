#ifndef CHART_EMA_H_
#define CHART_EMA_H_

#include <chart/data.h>

namespace chart {
namespace indicators {

data::PPoints ema(data::PPoints, size_t period);
data::PPoints forceIndex(data::PBars, size_t period);

struct Macd {
    data::PPoints slowEma;
    data::PPoints fastEma;
    data::PPoints macd;
    data::PPoints signal;
    data::PPoints histogram;
};

std::shared_ptr<Macd> macd(data::PPoints, size_t fastPeriod, size_t slowPeriod, size_t signalPeriod);

}}

#endif
