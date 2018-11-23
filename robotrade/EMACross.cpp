#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>
#include <chart/indicators.h>
#include <chart/stepFind.h>
#include <util/stream.h>
#include <util/hasher.h>

using namespace std;
using namespace util;
using namespace chart;

namespace robotrade {

const size_t emaFrom = 10;
const size_t emaTo = 100;

EMACross::EMACross(unsigned verbose) : verbose_(verbose)  {
}

void EMACross::process(
	bool printSummary, bool printOrders,
	unsigned daysToAnalyze, const std::vector<std::string> &quoteFiles, unsigned seed
) {
	Config config;
	if (quoteFiles.empty())
		return;

	auto priceInfos = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				Stream<ifstream> ifs(quoteFile.c_str());
				PriceInfo priceInfo;
				priceInfo.bars = robotrade::parse(ifs);
				priceInfo.emas.reserve(emaTo - emaFrom + 1);
				priceInfo.atrs.reserve(emaTo - emaFrom + 1);
				for (auto period = emaFrom; period <= emaTo; ++period) {
					priceInfo.emas.insert({
						period,
						indicators::ema(
							data::createPoints(priceInfo.bars, [&](size_t barNum){return priceInfo.bars->close(barNum);}),
							period
						)
					});
					priceInfo.atrs.insert({
						period,
						indicators::atr(priceInfo.bars, period)
					});

				}
				return priceInfo;
			};
		}
	);

	vector<TaskParam> taskParams;
	for (const auto &priceInfo: priceInfos) {
		auto startFrom = static_cast<size_t>(config.windowSizeK * emaTo);
		if (daysToAnalyze != 0)
			startFrom = max(startFrom, priceInfo.bars->num() - daysToAnalyze + 1);
		for (size_t barNum = startFrom; barNum <= priceInfo.bars->num(); ++barNum)
			taskParams.push_back({barNum, priceInfo});
	}

	const auto results = async_.execTasks(
		funcIterator(taskParams),
		[&] (const auto &taskParam) {
			return [&, taskParam] {
				return runTask(taskParam, seed, config);
			};
		}
	);

	if (verbose_ >= Log) {
		cout << "Log:" << endl;
		for (const auto &result: results)
			cout << result.log << endl;
	}

	if (printOrders) {
		for (const auto &result: results) {
			if (!result.results.empty())
				cout << result.title << endl;
			for (const auto &resultIt: result.results)
				cout << resultIt << endl;
		}
	}

	if (printSummary) {
		struct Summary {
			size_t numProfits = 0;
			size_t numLosses = 0;
		};

		const auto finRes = [&] (const Summary &summary)  {
			return -static_cast<double>(summary.numLosses) + config.profitPerStopK * summary.numProfits;
		};

		util::umap<string, Summary> summary;
		for (const auto &result: results) {
			auto &summ = summary[result.title];
			for (const auto &resultIt: result.results)
				if (resultIt.profit)
					++summ.numProfits;
				else if (resultIt.stopped && !resultIt.stopped->lossless)
					++summ.numLosses;
		}

		std::vector<decltype(summary.begin())> byFinRes;
		byFinRes.reserve(summary.size());
		for (auto it = summary.begin(); it != summary.end(); ++it)
			byFinRes.push_back(it);
		sort(byFinRes.begin(), byFinRes.end(), [&](const auto &l, const auto &r){return finRes(l->second) > finRes(r->second);});

		cout << "#\ttitle\t\tprofits\tlosses\tfinRes\n";
		size_t num = 0;
		size_t numProfits = 0;
		size_t numLosses = 0;
		double finResult = 0;
		for (const auto &x: byFinRes) {
			cout
				<< ++num << "\t"
				<< setw(15) << left << x->first << "\t"
				<< x->second.numProfits << "\t"
				<< x->second.numLosses << "\t"
				<< finRes(x->second) << endl;

			numProfits += x->second.numProfits;
			numLosses += x->second.numLosses;
			finResult += finRes(x->second);
		}
		cout
			<< endl
			<< '\t'
			<< setw(15) << left << "Total" << '\t'
			<< numProfits << '\t'
			<< numLosses << '\t'
			<< finResult
			<< endl;
	}
}

EMACross::TaskResult EMACross::runTask(
	const TaskParam &param, unsigned seed, const Config &config
) {
	ostringstream os;
	ProbabilityProvider probabilityProvider;
	EntryAnalyzer entryAnalyzer({}, param.priceInfo.bars, probabilityProvider, os, seed);
	EMACross::TaskResult result;
	result.title = param.priceInfo.bars->title(0);
	os << result.title << endl;
	const auto &bars = param.priceInfo.bars;
	for (auto period = emaTo; period >= emaFrom; os << endl, --period) {
		const auto &ema = param.priceInfo.emas.at(period);
		const auto &atr = param.priceInfo.atrs.at(period);

		const auto barFrom = param.barNum - static_cast<size_t>(period * config.windowSizeK);
		const auto lastBarNum = param.barNum - 1;
		const auto step = stepFind(bars, barFrom, param.barNum);
		os
			<< "ema " << period
			<< " from " << param.priceInfo.bars->time(barFrom)
			<< " to " << param.priceInfo.bars->time(lastBarNum);

		const auto lastCross =
			bars->low(lastBarNum) <= ema->close(lastBarNum) &&
			bars->high(lastBarNum) >= ema->close(lastBarNum);
		if (!lastCross) {
			os << " no last cross";
			continue;
		}

		size_t numBarsBelow = 0;
		size_t numBarsAbove = 0;
		for (auto barNum = lastBarNum; barNum >= barFrom; --barNum) {
			if (bars->low(barNum) > ema->close(barNum))
				++numBarsAbove;
			if (bars->high(barNum) < ema->close(barNum))
				++numBarsBelow;

			if (numBarsAbove && numBarsBelow)
				break;
		}

		if (numBarsAbove && numBarsBelow) {
			os << " bars above and below ema";
			continue;
		}

		if (numBarsAbove) {
			const auto stop = bars->low(lastBarNum);
			auto enter = ema->close(lastBarNum);
			enter = ceil(enter / step) * step;
			const auto move = (enter - stop) * config.profitPerStopK;
			if (move > config.maxMovePerAtrK * atr->close(lastBarNum)) {
				os << " target too far up";
				continue;
			}

			os
				<< endl
				<< "BUY "
				<< result.title
				<< " step " << setprecision(10) << step
				<< " period " << period
				<< " ema " << ema->close(lastBarNum) << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Buy,
				enter, stop, enter + move, lastBarNum
			));
			os << result.results.back();
			break;
		} else {
			const auto stop = bars->high(lastBarNum);
			auto enter = ema->close(lastBarNum);
			enter = floor(enter / step) * step;

			const auto move = (stop - enter) * config.profitPerStopK;
			if (move > config.maxMovePerAtrK * atr->close(lastBarNum)) {
				os << " target too far down";
				continue;
			}

			os
				<< endl
				<< "SELL "
				<< result.title
				<< " step " << setprecision(10) << step
				<< " period " << period
				<< " ema " << ema->close(lastBarNum) << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Sell,
				enter, stop, enter - move, lastBarNum
			));
			os << result.results.back();
			break;
		}
	}
	result.log = os.str();
	return result;
}

}
