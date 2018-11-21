#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>
#include <chart/indicators.h>
#include <util/stream.h>
#include <util/hasher.h>

using namespace std;
using namespace util;
using namespace chart;

namespace robotrade {

const size_t emaFrom = 10;
const size_t emaTo = 100;

EMACross::EMACross() {
}

void EMACross::process(const std::vector<std::string> &quoteFiles) {
	if (quoteFiles.empty())
		return;

	auto parseds = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				Stream<ifstream> ifs(quoteFile.c_str());
				Parsed parsed;
				parsed.bars = robotrade::parse(ifs);
				parsed.emas.reserve(emaTo - emaFrom + 1);
				parsed.atrs.reserve(emaTo - emaFrom + 1);
				for (auto period = emaFrom; period <= emaTo; ++period) {
					parsed.emas.insert({
						period,
						indicators::ema(
							data::createPoints(parsed.bars, [&](size_t barNum){return parsed.bars->close(barNum);}),
							period
						)
					});
					parsed.atrs.insert({
						period,
						indicators::atr(parsed.bars, period)
					});

				}
				return parsed;
			};
		}
	);

	cout << "Parsed " << parseds.size() << endl;

	vector<TaskParams> taskParams;
	for (const auto &parsed: parseds)
		for (size_t barNum = 2*emaTo; barNum < parsed.bars->num(); ++barNum)
			taskParams.push_back({barNum, parsed});

	const auto results = async_.execTasks(
		funcIterator(taskParams),
		[&] (const auto &it) {
			return [&, parsed = it.parsed, barNum = it.barNum] {
				return runTask({barNum, parsed});
			};
		}
	);

	cout << "Results:" << endl;
	for (const auto &result: results)
		cout << result.log << endl;

	struct S {
		size_t numProfits = 0;
		size_t numLosses = 0;
		auto finRes() const {return -static_cast<double>(numLosses) + 3*numProfits;}
	};
	util::umap<string, S> summary;
	for (const auto &result: results) {
		auto &summ = summary[result.title];
		for (const auto &x: result.results)
			if (x.profit)
				++summ.numProfits;
			else if (x.stopped && !x.stopped->lossless)
				++summ.numLosses;
	}

	std::vector<decltype (summary.begin())> byFinRes;
	byFinRes.reserve(summary.size());
	for (auto it = summary.begin(); it != summary.end(); ++it)
		byFinRes.push_back(it);
	sort(byFinRes.begin(), byFinRes.end(), [](const auto &l, const auto &r){return l->second.finRes() > r->second.finRes();});

	cout << "title\t\tlosses\tprofits\tfinRes\n";
	for (const auto &x: byFinRes) {
		cout
			<< setw(15) << left << x->first << "\t"
			<< x->second.numLosses << "\t"
			<< x->second.numProfits << "\t"
			<< x->second.finRes() << endl;
	}
}

EMACross::TaskResult EMACross::runTask(const TaskParams &params) {
	ostringstream os;
	ProbabilityProvider probabilityProvider;
	EntryAnalyzer entryAnalyzer({}, params.parsed.bars, probabilityProvider, os);
	EMACross::TaskResult result;
	result.title = params.parsed.bars->title(0);
	os << result.title << endl;
	for (auto emaPeriod = emaTo; emaPeriod >= emaFrom; --emaPeriod) {
		os
			<< "\nema " << emaPeriod
			<< " from " << params.parsed.bars->time(params.barNum - emaPeriod * 2)
			<< " to " << params.parsed.bars->time(params.barNum - 1);

		const auto barCrossesEMA = [&](size_t barNum) {
			return
				params.parsed.bars->low(barNum) <= params.parsed.emas.at(emaPeriod)->close(barNum) &&
				params.parsed.bars->high(barNum) >= params.parsed.emas.at(emaPeriod)->close(barNum);
		};

		const auto lastBarNum = params.barNum - 1;
		if (!barCrossesEMA(lastBarNum)) {
			os << " no last cross";
			continue;
		}

		size_t numBarsBelow = 0;
		size_t numBarsAbove = 0;
		for (auto barNum = lastBarNum; barNum >= params.barNum - emaPeriod * 2; --barNum) {
			if (params.parsed.bars->low(barNum) > params.parsed.emas.at(emaPeriod)->close(barNum))
				++numBarsAbove;
			if (params.parsed.bars->high(barNum) < params.parsed.emas.at(emaPeriod)->close(barNum))
				++numBarsBelow;

			if (numBarsAbove && numBarsBelow)
				break;
		}

		if (numBarsAbove && numBarsBelow) {
			os << " middle cross";
			continue;
		}

		// последняя свечка пересекает ema, другие нет

		if (numBarsAbove) {
			// пересечение сверху
			const auto stop = params.parsed.bars->low(lastBarNum);
			const auto enter = params.parsed.emas.at(emaPeriod)->close(lastBarNum);
			const auto move = (enter - stop) * 3;
			if (move > 2 * params.parsed.atrs.at(emaPeriod)->close(lastBarNum)) {
				os << " target too far down";
				continue;
			}

			os << " BUY " << emaPeriod << ' ' << result.title << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Buy,
				enter, stop, enter + move, lastBarNum, 0
			));
			os << result.results.back();
			break;
		} else {
			// пересечение сверху
			const auto stop = params.parsed.bars->high(lastBarNum);
			const auto enter = params.parsed.emas.at(emaPeriod)->close(lastBarNum);
			const auto move = (stop - enter) * 3;
			if (move > 2 * params.parsed.atrs.at(emaPeriod)->close(lastBarNum)) {
				os << " target too far low";
				continue;
			}

			os << " SELL " << emaPeriod << ' ' << result.title << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Sell,
				enter, stop, enter - move, lastBarNum, 0
			));
			os << result.results.back();
			break;
		}
	}
	result.log = os.str();
	return result;
}

}
