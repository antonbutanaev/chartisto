#ifndef CHART_EMA_H_
#define CHART_EMA_H_

#include <chart/data.h>

namespace chart {

data::PPoints ema(const data::PPoints&, size_t period);

}

#endif
