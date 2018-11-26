#include <json/json.h>
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
#include <util/stream.h>


using namespace std;
using namespace util;
using namespace chart;

namespace robotrade {

EMACross::Config::Risk EMACross::Config::getRisk(const std::string &title) const {
	Risk result;
	if (risk.isObject()) {
		const auto fill = [&](const string &key) {
			if (risk.isMember(key)) {
				const auto &value = risk[key];
				if (value.isMember("maxLoss"))
					result.maxLoss = value["maxLoss"].asDouble();
				if (value.isMember("maxPosition"))
					result.maxPosition = value["maxPosition"].asDouble();
			}
		};
		fill("default");
		fill(title);
	}
	return result;
}

EMACross::EMACross(const std::string &jsonConfig, unsigned verbose) : verbose_(verbose) {
	Stream<ifstream> ifs(jsonConfig);
	Json::Value configJson;
	ifs >> configJson;

	if (configJson.isMember("windowSizeK")) config_.windowSizeK = configJson["windowSizeK"].asDouble();
	if (configJson.isMember("profitPerStopK")) config_.profitPerStopK = configJson["profitPerStopK"].asDouble();
	if (configJson.isMember("maxMovePerAtrK")) config_.maxMovePerAtrK = configJson["maxMovePerAtrK"].asDouble();
	if (configJson.isMember("emaFrom")) config_.emaFrom = configJson["emaFrom"].asUInt();
	if (configJson.isMember("emaTo")) config_.emaTo = configJson["emaTo"].asUInt();
	if (configJson.isMember("risk")) config_.risk = configJson["risk"];

	cout
		<< "EMACross using params:"
		<< endl << "windowSizeK " << config_.windowSizeK
		<< endl << "profitPerStopK " << config_.profitPerStopK
		<< endl << "maxMovePerAtrK " << config_.maxMovePerAtrK
		<< endl << "emaFrom " << config_.emaFrom
		<< endl << "emaTo " << config_.emaTo
		<< endl;
}

void EMACross::process(
	bool printSummary, bool printOrders,
	unsigned daysToAnalyze, const std::vector<std::string> &quoteFiles, unsigned seed
) {
	if (quoteFiles.empty())
		return;

	auto priceInfos = async_.execTasks(
		funcIterator(quoteFiles),
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				Stream<ifstream> ifs(quoteFile.c_str());
				PriceInfo priceInfo;
				priceInfo.bars = robotrade::parse(ifs);
				priceInfo.emas.reserve(config_.emaTo - config_.emaFrom + 1);
				priceInfo.atrs.reserve(config_.emaTo - config_.emaFrom + 1);
				for (auto period = config_.emaFrom; period <= config_.emaTo; ++period) {
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
		auto startFrom = static_cast<size_t>(config_.windowSizeK * config_.emaTo);
		if (daysToAnalyze != 0)
			startFrom = max(startFrom, priceInfo.bars->num() - daysToAnalyze + 1);
		for (size_t barNum = startFrom; barNum <= priceInfo.bars->num(); ++barNum)
			taskParams.push_back({barNum, priceInfo});
	}

	auto results = async_.execTasks(
		funcIterator(taskParams),
		[&] (const auto &taskParam) {
			return [&, taskParam] {
				return runTask(taskParam, seed);
			};
		}
	);

	sort(results.begin(), results.end(), [](const auto &l, const auto &r){
		return l.title < r.title;
	});

	if (verbose_ >= Log) {
		cout << "Log:" << endl;
		for (const auto &result: results)
			cout << result.log << endl;
	}

	if (printOrders) {
		string prevTitle;
		for (const auto &result: results) {
			if (result.orders.empty())
				continue;
			if (prevTitle.empty() || prevTitle != result.title)
				cout << result.title << endl;
			prevTitle = result.title;
			for (const auto &order: result.orders)
				cout
					<< "ema " << order.period
					<< " " << order.result
					<< endl;
		}
	}

	if (printSummary) {
		struct Summary {
			size_t numProfits = 0;
			size_t numLosses = 0;
		};

		const auto finRes = [&] (const Summary &summary)  {
			return -static_cast<double>(summary.numLosses) + config_.profitPerStopK * summary.numProfits;
		};

		util::umap<string, Summary> summary;
		for (const auto &result: results) {
			auto &summ = summary[result.title];
			for (const auto &order: result.orders)
				if (order.result.profit)
					++summ.numProfits;
				else if (order.result.stopped && !order.result.stopped->lossless)
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
	const TaskParam &param, unsigned seed
) {
	ostringstream os;
	ProbabilityProvider probabilityProvider;
	EntryAnalyzer entryAnalyzer({}, param.priceInfo.bars, probabilityProvider, os, seed);
	EMACross::TaskResult result;
	result.title = param.priceInfo.bars->title(0);
	const auto &bars = param.priceInfo.bars;
	const auto risk = config_.getRisk(bars->title(0));
	os
		<< result.title
		<< " max position " << risk.maxPosition
		<< " max loss " << risk.maxLoss
		<< endl;
	const auto findBarFrom = [&](size_t period) {return param.barNum - static_cast<size_t>(period * config_.windowSizeK);};
	const auto step = stepFind(bars, findBarFrom(config_.emaTo), param.barNum);
	for (auto period = config_.emaTo; period >= config_.emaFrom; os << endl, --period) {
		const auto &ema = param.priceInfo.emas.at(period);
		const auto &atr = param.priceInfo.atrs.at(period);

		const auto barFrom = findBarFrom(period);
		const auto lastBarNum = param.barNum - 1;
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
		for (auto barNum = lastBarNum-1; barNum >= barFrom; --barNum) {
			if (bars->low(barNum) > ema->close(barNum))
				++numBarsAbove;
			else if (bars->high(barNum) < ema->close(barNum))
				++numBarsBelow;

			if (numBarsAbove && numBarsBelow)
				break;
		}

		if (numBarsAbove && numBarsBelow) {
			os << " bars above and below ema";
			continue;
		}


		if (numBarsAbove) {
			auto stop = bars->low(lastBarNum) - 2 * step;
			auto enter = ema->close(lastBarNum);
			enter = (-1. + ceil(enter / step)) * step;
			const auto stop2 = enter * (1 - risk.maxLoss / risk.maxPosition);
			if (stop2 < stop) {
				os << " move stop from " << stop << " to " << stop2 << " enter " << enter;
				stop = stop2;
			}

			const auto move = (enter - stop) * config_.profitPerStopK;
			if (move > config_.maxMovePerAtrK * atr->close(lastBarNum)) {
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
			result.orders.push_back({period, entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Buy,
				enter, stop, enter + move, lastBarNum
			)});
			os << result.orders.back().result;
			break;
		} else {
			auto stop = bars->high(lastBarNum) + 2*step;
			auto enter = ema->close(lastBarNum);
			enter = (1. + floor(enter / step)) * step;
			auto stop2 = enter * (1 + risk.maxLoss / risk.maxPosition);
			if (stop2 > stop) {
				os << " move stop from " << stop << " to " << stop2 << " enter " << enter;
				stop = stop2;
			}
			if (stop2 > stop) {
				os << " move stop from " << stop << " to " << stop2;
				stop = stop2;
			}

			const auto move = (stop - enter) * config_.profitPerStopK;
			if (move > config_.maxMovePerAtrK * atr->close(lastBarNum)) {
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
			result.orders.push_back({period, entryAnalyzer.analyze(
				EntryAnalyzer::Direction::Sell,
				enter, stop, enter - move, lastBarNum
			)});
			os << result.orders.back().result;
			break;
		}
	}
	result.log = os.str();
	return result;
}

}
