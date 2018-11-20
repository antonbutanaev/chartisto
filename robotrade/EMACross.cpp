#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>
#include <chart/indicators.h>
#include <util/stream.h>
#include <util/hashes.h>

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
		umap<size_t, data::PPoints> emas;
		umap<size_t, data::PPoints> atrs;
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
				atr = it.second->atrs.at(it.first),
				bars = it.second->bars
			] {
				return runTask({emaPeriod, ema, atr, bars});
			};
		}
	);

	if ((0)) {
	cout << "Results:" << endl;
	for (const auto &result: results)
		cout << result.log << endl;
	}

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
	EntryAnalyzer entryAnalyzer({}, params.bars, probabilityProvider, os);
	EMACross::TaskResult result;
	result.title = params.bars->title(0);
	for (
		auto barsFrom = params.emaPeriod, barsTo = barsFrom + 2 * params.emaPeriod;
		barsTo <= params.bars->num();
		++barsFrom, ++barsTo
	 ) {
		os
			<< "\nfrom " << params.bars->time(barsFrom)
			<< " to " << params.bars->time(barsTo - 1);

		const auto barCrossesEMA = [&](size_t barNum) {
			return
				params.bars->low(barNum) <= params.ema->close(barNum) &&
				params.bars->high(barNum) >= params.ema->close(barNum);
		};

		const auto lastBarNum = barsTo - 1;
		if (!barCrossesEMA(lastBarNum)) {
			os << " no last cross";
			continue;
		}

		size_t numBarsBelow = 0;
		size_t numBarsAbove = 0;
		for (auto barNum = barsFrom; barNum != lastBarNum; ++barNum) {
			if (params.bars->low(barNum) > params.ema->close(barNum))
				++numBarsAbove;
			else if (params.bars->high(barNum) < params.ema->close(barNum))
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
			const auto stop = params.bars->low(lastBarNum);
			const auto enter = params.ema->close(lastBarNum);
			const auto move = (enter - stop) * 3;
			if (move > 2 * params.atr->close(lastBarNum)) {
				os << " target too far up";
				continue;
			}

			os << " BUY ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Buy,
				enter, stop, enter + move, lastBarNum, 0
			));
			os << result.results.back();
		} else {
			// пересечение сверху
			const auto stop = params.bars->high(lastBarNum);
			const auto enter = params.ema->close(lastBarNum);
			const auto move = (stop - enter) * 3;
			if (move > 2 * params.atr->close(lastBarNum)) {
				os << " target too far low";
				continue;
			}

			os << " SELL ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Sell,
				enter, stop, enter - move, lastBarNum, 0
			));
			os << result.results.back();
		}
	}
	result.log = os.str();
	return result;
}

}
