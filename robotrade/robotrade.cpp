#include <iostream>
#include <fstream>
#include <json/json.h>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include <robotrade/quotesparser.h>

#include "Levels.h"
#include "triplescreen.h"

using namespace robotrade;
using namespace date;

using namespace std;

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argQuotes = "quotes",
		*argLevelsJson = "levels",
		*argLog = "log";

	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<string>(), "file with quotes")
		(argLevelsJson, po::value<string>()->default_value("levels.json"), "levels .json file")
		(argLog, po::value<string>()->default_value("robotrade_log.conf"), "log .conf file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure(vm["log"].as<string>());

	auto logger = log4cplus::Logger::getInstance("main");
	LOG4CPLUS_DEBUG(logger, "start");

	if (vm.count(argHelp)) {
		cout << desc << endl;
		return 1;
	}

	if (vm.count(argQuotes)) {
		const auto quotes = vm[argQuotes].as<string>();
		ifstream ifs(quotes.c_str());
		if (!ifs)
			throw runtime_error("Could not open file: " + quotes);

		const auto bars = robotrade::parse(ifs);
		if (vm.count(argLevelsJson)) {
			findLevels(bars, 0, bars->num(), vm[argLevelsJson].as<string>());
		} else
			throw runtime_error("What to do with quotes?");
	}
	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
