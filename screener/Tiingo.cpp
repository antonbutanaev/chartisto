#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include "ProcessTickers.h"

using namespace std;
using namespace tiingo;

int main(int ac, char** av) try {
	const char
		*argHelp = "help",
		*argTickers = "tickers",
		*argAuthToken = "auth-token",
		*argQuotesDir = "quotes-dir";

	namespace po = boost::program_options;

	po::options_description description("Update Tiingo cache");
	description.add_options()
		(argHelp, "Help message")
		(argQuotesDir, po::value<string>(), "Directory with Tiingo cache")
		(argTickers, po::value<string>(), "Ticker list file")
		(argAuthToken, po::value<string>(), "Tiingo auth token")
		;

	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av)
			.options(description)
			.run(),
		vm
	);
	po::notify(vm);

	if (vm.count(argHelp)) {
		cout
			<< "Hardware concurrency: " << thread::hardware_concurrency() << endl
			<< description << endl;
		return 1;
	}

	ifstream tickers(vm[argTickers].as<string>());
	if (!tickers) {
		cerr << "Could not open " << vm[argTickers].as<string>() << endl;
		return 1;
	}

	processTickers(tickers, vm[argQuotesDir].as<string>());
} catch (const exception &x) {
	cerr << "exception "  << x.what() << endl;
	return 1;
}
