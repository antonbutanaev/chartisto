#ifndef CHART_REDUCE_H_
#define CHART_REDUCE_H_

#include <functional>
#include <chart/data.h>

namespace chart {

using ReduceFunc = std::function<Time(Time)>;
data::PBars reduce(const data::Bars &bars, const ReduceFunc &reduceFunc);
Time weekReduce(Time);

}

#endif
