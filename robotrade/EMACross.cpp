#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>
#include <chart/indicators.h>

using namespace std;
using namespace util;
using namespace chart;

namespace robotrade {

EMACross::EMACross() {
}

void EMACross::process(const std::vector<std::string> &quoteFiles) {
	if (quoteFiles.empty())
		return;

	struct Bars {
		data::PBars bars;
		unordered_map<size_t, data::PPoints> emas;
	};

	const size_t emaFrom = 10;
	const size_t emaTo = 100;

	auto bars = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				ifstream ifs(quoteFile.c_str());
				if (!ifs)
					throw runtime_error("Could not open file: " + quoteFile);
				Bars bars;
				bars.bars = robotrade::parse(ifs);
				bars.emas.reserve(emaTo - emaFrom + 1);
				for (auto period = emaFrom; period <= emaTo; ++period)
					bars.emas.insert({
						period,
						indicators::ema(
							data::createPoints(bars.bars, [&](size_t barNum){return bars.bars->close(barNum);}),
							period
						)
					});
				return bars;
			};
		}
	);

	cout << "Parsed " << bars.size() << endl;

	const auto results = async_.execTasks(
		funcPairIterator(funcIterator(bars), funcRangeIterator(emaFrom, emaTo + 1)),
		[&] (const auto &params) {
			return [
				&,
				emaPeriod = params.second,
				ema = params.first.emas.at(params.second),
				bars = params.first.bars
			] {
				return runTask({emaPeriod, ema, bars});
			};
		}
	);

	cout << "Results:" << endl;
	for (const auto &result: results)
		cout << result.log << endl;
}

EMACross::TaskResult EMACross::runTask(const TaskParams &params) {
	ostringstream os;
	for (
		auto barsFrom = params.emaPeriod, barsTo = barsFrom + 2 * params.emaPeriod;
		barsTo < params.bars->num();
		++barsFrom, ++barsTo
	 ) {
		bool noCross = true;
		for (auto barNum = barsFrom; barNum != barsTo; ++barNum)
			if (
				params.bars->low(barNum) <= params.ema->close(barNum) &&
				params.bars->high(barNum) >= params.ema->close(barNum)
			) {
				noCross = false;
				break;
			}

		os
			<< "ema " << params.emaPeriod
			<< " bars " << params.bars->title(0)
			<< " from " << params.bars->time(barsFrom)
			<< " to " << params.bars->time(barsTo - 1)
			<< " noCross " << noCross << endl;
	}
	return {os.str()};
}

}
