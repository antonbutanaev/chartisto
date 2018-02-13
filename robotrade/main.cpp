#include <iostream>
#include <fstream>
#include <json/json.h>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include <robotrade/quotesparser.h>

using namespace std;

int main(int ac, char *av[]) try {
	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure("robotrade.ini");

	auto logger = log4cplus::Logger::getInstance("main");
	LOG4CPLUS_DEBUG(logger, "start");

	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("quotes", po::value<string>(), "file with quotes");

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);   

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	if (vm.count("quotes")) {
		const auto quotes = vm["quotes"].as<string>();
		ifstream ifs(quotes.c_str());
		if (!ifs)
			throw runtime_error("Could1 not open file: " + quotes);

		robotrade::parse(ifs);
	}

	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
