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

	auto ema = emaFrom;
	auto barsFrom = ema;
	auto barsTo = barsFrom + 2*ema;
	auto barsIt = bars.begin();

	auto paramsIter = [&] {
		optional<TaskParams> ret;
		if (barsTo < barsIt->bars->num())
			ret = {barsIt->emas[ema], barsIt->bars, barsFrom++, barsTo++};
		else if (++ema <= emaTo)
			ret = {barsIt->emas[ema], barsIt->bars, barsFrom = ema, barsTo = barsFrom + 2*ema};
		else if (++barsIt != bars.end()) {
			ret = {barsIt->emas[ema = emaFrom], barsIt->bars, barsFrom = ema, barsTo = barsFrom + 2*ema};
		}
		return ret;
	};

	const auto results = async_.execTasks(
		paramsIter,
		[&] (const TaskParams &params) {
			return [&, params] {return runTask(params);};
		}
	);

	cout << "Results:" << endl;
	for (const auto &result: results)
		cout << result.log << endl;
}

EMACross::TaskResult EMACross::runTask(const TaskParams &params) {
	bool noCross = true;
	for (auto barNum = params.barsFrom; barNum != params.barsTo; ++barNum)
		if (
			params.bars->low(barNum) <= params.ema->close(barNum) &&
			params.bars->high(barNum) >= params.ema->close(barNum)
		) {
			noCross = false;
			break;
		}

	ostringstream os;
	os
		<< "ema " << params.ema
		<< " bars " << params.bars->title(0)
		<< " from " << params.bars->time(params.barsFrom)
		<< " to " << params.bars->time(params.barsTo - 1)
		<< " noCross " << noCross
		;
	return {os.str()};
}

}
