#include <iostream>
#include <sstream>
#include <fstream>
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

	auto bars = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				ifstream ifs(quoteFile.c_str());
				if (!ifs)
					throw runtime_error("Could not open file: " + quoteFile);
				return robotrade::parse(ifs);
			};
		}
	);

	cout << "Parsed " << bars.size() << endl;

	const size_t emaFrom = 10;
	const size_t emaTo = 100;

	auto ema = emaFrom;
	auto barsFrom = ema;
	auto barsTo = barsFrom + 2*ema;
	auto barsIt = bars.begin();

	auto paramsIter = [&] {
		optional<TaskParams> ret;
		if (barsIt != bars.end()) {
			if (barsTo < (*barsIt)->num())
				ret = {ema, *barsIt, barsFrom++, barsTo++};
			else if (ema <= emaTo)
				ret = {ema++, *barsIt, barsFrom=ema, barsTo=barsFrom+2*ema};
			else
				ret = {ema=emaFrom, *barsIt++, barsFrom=ema, barsTo=barsFrom+2*ema};
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
	const auto ema = indicators::ema(
		data::createPoints(params.bars, [&](size_t barNum){return params.bars->close(barNum);}),
		params.ema
	);

	bool noCross = true;
	for (auto barNum = params.barsFrom; barNum != params.barsTo; ++barNum)
		if (
			params.bars->low(barNum) <= ema->close(barNum) &&
			params.bars->high(barNum) >= ema->close(barNum)
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
