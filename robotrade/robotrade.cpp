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
		}
	);

	const auto result = tripleScreen.run();
	cout
		<< "time,number,enterPrice,stopPrice,stopTime,maxProfitToStop,barsToStop\n";
	const auto optional = [](auto value, auto no, auto sep) {
		if (value == no)
			cout << sep;
		else
			cout << value << sep;
	};
	for (const auto &trade: result.trades) {
		cout
			<< trade.time << ','
			<< trade.number << ','
			<< trade.enterPrice << ','
			<< trade.stopPrice << ',';
		optional(trade.stoppedTime, NoTime, ',');
		optional(trade.maxProfitToStop, NoPrice, ',');
		optional(trade.barsToStop, StrategyResult::Trade::NoStop, '\n');
	}
}

int main(int ac, char *av[]) try {

	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("quotes", po::value<string>(), "file with quotes")
		("log", po::value<string>(), "log cong")
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
			throw runtime_error("Could1 not open file: " + quotes);

		runTripleScreen(robotrade::parse(ifs));
	}

	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
