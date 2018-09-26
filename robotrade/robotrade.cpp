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
	Price precisionK = 0.002;
	Price step = 0.01;
	double touchWeight = 3;
	double crossWeight = -1;
	double sameLevelK = 0.03;
	size_t maxlevels = 7;
};

void findLevels(data::PBars bars, const FindLevelsParams & params) {
	auto minPrice = bars->close(bars->num()-1) * (1 - params.priceRangeK);
	auto maxPrice = bars->close(bars->num()-1) * (1 + params.priceRangeK);
	minPrice = ceil(minPrice / params.step) * params.step;
	maxPrice = floor(maxPrice / params.step) * params.step;

	cout << "Price range: " << minPrice << " " << maxPrice << endl;

	const auto barTouchesPrice = [&](size_t barNum, Price price) {
		for (const auto &priceType: data::Bars::PriceTypes) {
			const auto diff = fabs(1 - price/bars->get(priceType, barNum));
			if (diff < params.precisionK)
				return true;
		}
		return false;
	};

	struct Level {
		size_t numTouches;
		size_t numBodyCrosses;
		Price level;
	};

	std::vector<Level> levels;

	for (auto price = minPrice; price <= maxPrice; price += params.step) {
		Level level{0, 0, price};
		for (size_t i = 0; i < bars->num(); ++i) {
			if (barTouchesPrice(i, price))
				level.numTouches += 1;

			if (
				(bars->open(i) > price && bars->close(i) < price) ||
				(bars->open(i) < price && bars->close(i) > price)
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
		findLevels(robotrade::parse(ifs), {});
	}

	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
