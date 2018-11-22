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

struct EMACrossConfig {
	double windowSizeK = 2.;
	double profitPerStopK = 3.;
};

EMACross::EMACross() {
}

void EMACross::process(const std::vector<std::string> &quoteFiles, unsigned seed) {
	EMACrossConfig config;
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
	for (const auto &priceInfo: priceInfos)
		for (
			size_t barNum = static_cast<size_t>(config.windowSizeK * emaTo);
			barNum < priceInfo.bars->num();
			++barNum
		)
			taskParams.push_back({barNum, priceInfo});

	const auto results = async_.execTasks(
		funcIterator(taskParams),
		[&] (const auto &taskParam) {
			return [&, taskParam] {
				return runTask(taskParam, seed);
			};
		}
	);

	cout << "Log:" << endl;
	for (const auto &result: results)
		cout << result.log << endl;

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

EMACross::TaskResult EMACross::runTask(const TaskParam &params, unsigned seed) {
	ostringstream os;
	ProbabilityProvider probabilityProvider;
	EntryAnalyzer entryAnalyzer({}, params.priceInfo.bars, probabilityProvider, os);
	EMACross::TaskResult result;
	result.title = params.priceInfo.bars->title(0);
	os << result.title << endl;
	for (auto emaPeriod = emaTo; emaPeriod >= emaFrom; --emaPeriod) {
		const auto barFrom = params.barNum - emaPeriod * 2;
		const auto lastBarNum = params.barNum - 1;
		os
			<< "\nema " << emaPeriod
			<< " from " << params.priceInfo.bars->time(barFrom)
			<< " to " << params.priceInfo.bars->time(lastBarNum);

		const auto barCrossesEMA = [&](size_t barNum) {
			return
				params.priceInfo.bars->low(barNum) <= params.priceInfo.emas.at(emaPeriod)->close(barNum) &&
				params.priceInfo.bars->high(barNum) >= params.priceInfo.emas.at(emaPeriod)->close(barNum);
		};

		if (!barCrossesEMA(lastBarNum)) {
			os << " no last cross";
			continue;
		}

		size_t numBarsBelow = 0;
		size_t numBarsAbove = 0;
		for (auto barNum = lastBarNum; barNum >= barFrom; --barNum) {
			if (params.priceInfo.bars->low(barNum) > params.priceInfo.emas.at(emaPeriod)->close(barNum))
				++numBarsAbove;
			if (params.priceInfo.bars->high(barNum) < params.priceInfo.emas.at(emaPeriod)->close(barNum))
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
			const auto stop = params.priceInfo.bars->low(lastBarNum);
			const auto enter = params.priceInfo.emas.at(emaPeriod)->close(lastBarNum);
			const auto move = (enter - stop) * 3;
			if (move > 2 * params.priceInfo.atrs.at(emaPeriod)->close(lastBarNum)) {
				os << " target too far down";
				continue;
			}

			os << " BUY " << emaPeriod << ' ' << result.title << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Buy,
				enter, stop, enter + move, lastBarNum, seed
			));
			os << result.results.back();
			break;
		} else {
			// пересечение сверху
			const auto stop = params.priceInfo.bars->high(lastBarNum);
			const auto enter = params.priceInfo.emas.at(emaPeriod)->close(lastBarNum);
			const auto move = (stop - enter) * 3;
			if (move > 2 * params.priceInfo.atrs.at(emaPeriod)->close(lastBarNum)) {
				os << " target too far low";
				continue;
			}

			os << " SELL " << emaPeriod << ' ' << result.title << " ";
			result.results.push_back(entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Sell,
				enter, stop, enter - move, lastBarNum, seed
			));
			os << result.results.back();
			break;
		}
	}
	result.log = os.str();
	return result;
}

}
