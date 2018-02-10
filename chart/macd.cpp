#include <vector>
#include <chart/macd.h>
#include <chart/ema.h>

using namespace std;

namespace chart {

shared_ptr<Macd> macd(data::PPoints points, size_t fastPeriod, size_t slowPeriod, size_t signalPeriod) {
    auto result = make_shared<Macd>();
    result->fastEma = ema(points, fastPeriod);
    result->slowEma = ema(points, slowPeriod);
    result->macd = data::convertPoints(result->fastEma, [&] (size_t n) {return result->fastEma->close(n) - result->slowEma->close(n);});
    result->signal = ema(result->macd, signalPeriod);
    result->histogram = data::convertPoints(result->fastEma, [&] (size_t n) {return result->macd->close(n) - result->signal->close(n);});
    return result;
}

}
