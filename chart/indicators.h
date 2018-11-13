#ifndef CHART_EMA_H_
#define CHART_EMA_H_

#include <chart/data.h>

namespace chart {
namespace indicators {

data::PPoints ema(data::PPoints, size_t period);
data::PPoints forceIndex(data::PBars, size_t period);
data::PPoints atr(data::PBars, size_t period);

struct Macd {
    data::PPoints slowEma, fastEma, macd, signal, histogram;
};

std::shared_ptr<Macd> macd(data::PPoints, size_t fastPeriod, size_t slowPeriod, size_t signalPeriod);

}}

#endif
