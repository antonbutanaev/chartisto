#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>
#include <chart/indicators.h>
#include <util/stream.h>

using namespace std;
using namespace util;
using namespace chart;

namespace robotrade {

EMACross::EMACross() {
}

void EMACross::process(const std::vector<std::string> &quoteFiles) {
	if (quoteFiles.empty())
		return;

	struct Parsed {
		data::PBars bars;
		unordered_map<size_t, data::PPoints> emas;
	};

	const size_t emaFrom = 10;
	const size_t emaTo = 100;

	auto parsed = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				Stream<ifstream> ifs(quoteFile.c_str());
				Parsed parsed;
				parsed.bars = robotrade::parse(ifs);
				parsed.emas.reserve(emaTo - emaFrom + 1);
				for (auto period = emaFrom; period <= emaTo; ++period)
					parsed.emas.insert({
						period,
						indicators::ema(
							data::createPoints(parsed.bars, [&](size_t barNum){return parsed.bars->close(barNum);}),
							period
						)
					});
				return parsed;
			};
		}
	);

	cout << "Parsed " << parsed.size() << endl;

	const auto results = async_.execTasks(
		funcPairIterator(
			funcRangeIterator(emaFrom, emaTo + 1),
			funcRangeIterator(parsed.begin(), parsed.end())
		),
		[&] (const auto &it) {
			return [&,
				emaPeriod = it.first,
				ema = it.second->emas.at(it.first),
				bars = it.second->bars
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
