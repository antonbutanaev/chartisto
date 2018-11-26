#include <cmath>
#include <algorithm>
#include <chart/stepFind.h>

using namespace std;

namespace chart {

Price stepFind(data::PBars bars, size_t barFrom, size_t barTo) {
	Price step = bars->close(barFrom);
	for (size_t barNum = barFrom + 1; barNum < barTo; ++barNum) {
		for (const auto &a: data::Bars::PriceTypes)
			for (const auto &b: data::Bars::PriceTypes) {
				const auto updateStep = [&](size_t barNum1, size_t barNum2) {
					auto delta = fabs(bars->get(a, barNum1) - bars->get(b, barNum2));
					if (delta > PriceEpsilon)
						step = min(step, delta);
				};
				updateStep(barNum, barNum);
				updateStep(barNum, barNum - 1);
			}
	}
	return step;
}

Price roundUp(Price price, Price step) {
	return ceil(price / step) * step;
}

Price roundDown(Price price, Price step) {
	return floor(price / step) * step;
}


}
