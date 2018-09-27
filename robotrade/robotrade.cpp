#include <iostream>
#include <fstream>
#include <json/json.h>
#include <boost/program_options.hpp>
#include <chart/indicators.h>
#include <chart/reduce.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include <robotrade/quotesparser.h>
#include <robotrade/triplescreen.h>

using chart::indicators::macd;
using robotrade::TripleScreen;

using namespace std;
using namespace chart;
using namespace robotrade;

void runTripleScreen(data::PBars bars) {
	const auto barsDaily = reduce(*bars, dayReduce);
	const auto barsWeekly = reduce(*barsDaily, weekReduce);

	const auto closeWeekly = data::createPoints(barsWeekly, [&](size_t n) {return barsWeekly->close(n);});
	const auto macdWeekly = indicators::macd(closeWeekly, 13, 26, 7);

	const auto forceIndex = indicators::forceIndex(barsDaily, 2);

	const auto optional = [](auto value, auto no, auto sep) {
		if (value == no)
			cout << sep;
		else
			cout << value << sep;
	};

	Trader trader({
		100, 2000,
		[&](const Trader::OnTrade &trade) {
			cout
				<< trade.time << ','
				<< trade.num << ','
				<< trade.price << ',';
			optional(trade.gain, NoPrice, ',');
			cout
				<< trade.total << ','
				<< trade.pos << '\n';

		}
	});

	TripleScreen tripleScreen(
		barsWeekly, barsDaily,
		[&](size_t weekly, size_t daily) {
			if (weekly == 0)
			return Action::Wait;
			if (
				macdWeekly->histogram->close(weekly-1) < macdWeekly->histogram->close(weekly) &&
				macdWeekly->histogram->close(weekly) < 0 &&
				forceIndex->close(daily) < 0
			)
				return Action::Buy;
			else if (
				macdWeekly->histogram->close(weekly-1) > macdWeekly->histogram->close(weekly) &&
				macdWeekly->histogram->close(weekly) > 0 &&
				forceIndex->close(daily) > 0
			)
				return Action::Sell;
			else
				return Action::Wait;
		},
		trader
	);

	cout << "time,num,price,gain,total,position\n";
	tripleScreen.run();
	trader.trade({Trader::Trade::Close, barsDaily->time(barsDaily->num()-1), barsDaily->close(barsDaily->num()-1), NoPrice});
}

struct FindLevelsParams {
	float priceRangeK = 0.2;
	size_t minTouches = 3;
	Price precisionK = 0.001;
	Price step = 0.01;
	double touchWeight = 1;
	double crossWeight = -2;
	double sameLevelK = 0.03;
	size_t maxlevels = 10;
	size_t minExtremumAgeBars = 20;
	size_t extremumNumTouches = 100;
};

struct Level {
	size_t numTouches;
	size_t numBodyCrosses;
	Price level;
};

void findLevels(data::PBars bars, size_t from, size_t to, const FindLevelsParams & params) {
	std::vector<Level> levels;
	if (from >= to)
		return;

	auto minPrice = numeric_limits<Price>::max();
	auto maxPrice = numeric_limits<Price>::lowest();
	size_t minPriceNum = from;
	size_t maxPriceNum = from;
	for (auto barNum = from; barNum < to; ++barNum) {
		if (minPrice > bars->low(barNum)) {
			minPrice = bars->low(barNum);
			minPriceNum = barNum;
		}
		if (maxPrice < bars->high(barNum)) {
			maxPrice = bars->high(barNum);
			maxPriceNum = barNum;
		}
	}

	auto rangeLow = bars->close(to - 1) * (1 - params.priceRangeK);
	auto rangeHigh = bars->close(to - 1) * (1 + params.priceRangeK);
	rangeLow = ceil(rangeLow / params.step) * params.step;
	rangeHigh = floor(rangeHigh / params.step) * params.step;

	rangeLow = std::max(minPrice, rangeLow);
	rangeHigh = std::min(maxPrice, rangeHigh);

	cout << "Price range: " << minPrice << " " << maxPrice << endl;

	if (minPrice == rangeLow && minPriceNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			minPrice
		});

	if (maxPrice == rangeHigh && maxPriceNum + params.minExtremumAgeBars <= to)
		levels.push_back({
			params.extremumNumTouches,
			0,
			maxPrice
		});

	for (auto price = rangeLow; price <= rangeHigh; price += params.step) {
		Level level{0, 0, price};
		for (size_t barNum = from; barNum < to; ++barNum) {
			const auto upperBound = price * (1 + params.precisionK);
			const auto lowerBound = price * (1 - params.precisionK);

			for (const auto &priceType: data::Bars::PriceTypes) {
				const auto barPrice = bars->get(priceType, barNum);
				if (barPrice >= lowerBound && barPrice <= upperBound) {
					level.numTouches += 1;
					break;
				}
			}

			const auto barOpen = bars->open(barNum);
			const auto barClose = bars->close(barNum);
			if (
				(barOpen > upperBound && barClose < lowerBound) ||
				(barOpen < lowerBound && barClose > upperBound)
			)
				level.numBodyCrosses += 1;
		}
		if (level.numTouches >= params.minTouches)
			levels.push_back(level);
	}

	sort(
		levels.begin(), levels.end(),
		[&](const auto &a, const auto &b) {
			const auto rate = [&] (const auto &level) {
				return
					level.numTouches * params.touchWeight +
					level.numBodyCrosses * params.crossWeight;
			};

			return rate(a) > rate(b);
		}
	);

	for (auto levelIt = levels.begin(); levelIt != levels.end(); ) {
		bool levelRepeat = false;
		for (auto prevLevelIt = levels.begin(); prevLevelIt != levelIt; ++prevLevelIt) {
			if (fabs(1 - prevLevelIt->level / levelIt->level) < params.sameLevelK) {
				levelRepeat = true;
				break;
			}
		}
		if (levelRepeat)
			levelIt = levels.erase(levelIt);
		else
			++levelIt;
	}

	cout << "Compacted" << endl;
	size_t n = 0;
	for (const auto & level: levels) {
		if (++n > params.maxlevels)
			break;
		cout << level.numTouches << ' ' << level.numBodyCrosses << ' ' << level.level << endl;
	}
}

int main(int ac, char *av[]) try {
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("quotes", po::value<string>(), "file with quotes")
		("log", po::value<string>()->default_value("robotrade_log.conf"), "log .conf file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure(vm["log"].as<string>());

	auto logger = log4cplus::Logger::getInstance("main");
	LOG4CPLUS_DEBUG(logger, "start");

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	if (vm.count("quotes")) {
		const auto quotes = vm["quotes"].as<string>();
		ifstream ifs(quotes.c_str());
		if (!ifs)
			throw runtime_error("Could not open file: " + quotes);

		//runTripleScreen(robotrade::parse(ifs));
		const auto bars = robotrade::parse(ifs);
		findLevels(bars, 0, bars->num(), {});
	}

	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
