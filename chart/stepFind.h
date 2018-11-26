#pragma once

#include <chart/data.h>

namespace chart {

Price stepFind(data::PBars bars, size_t barFrom, size_t barTo);
Price roundUp(Price price, Price step);
Price roundDown(Price price, Price step);

}
