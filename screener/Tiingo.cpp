#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/program_options.hpp>
#include <date/date.h>
#include "ProcessTickers.h"

using namespace std;
using namespace tiingo;
using namespace date;

int main(int ac, char** av) try {
	const char
		*argHelp = "help",
		*argTickers = "tickers",
		*argAuthToken = "auth-token",
		*argFromDate = "from-date",
		*argQuotesDir = "quotes-dir";

	Date to = floor<days>(chrono::system_clock::now());
	Date from = to - years{2};

	stringstream fromStr;
	fromStr << from;

	namespace po = boost::program_options;

	po::options_description description("Update Tiingo cache");
	description.add_options()
		(argHelp, "Help message")
		(argQuotesDir, po::value<string>(), "Directory with Tiingo cache")
		(argFromDate, po::value<string>()->default_value(fromStr.str()), "From date")
		(argTickers, po::value<string>(), "Ticker list file")
		(argAuthToken, po::value<string>(), "Tiingo auth token file")
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
		cerr << "Could not open tickers file " << vm[argTickers].as<string>() << endl;
		return 1;
	}

	ifstream authToken(vm[argAuthToken].as<string>());
	if (!tickers) {
		cerr << "Could not open auth token file " << vm[argAuthToken].as<string>() << endl;
		return 1;
	}

	string authTokenStr;
	authToken >> authTokenStr;


	processTickers(tickers, vm[argQuotesDir].as<string>(), authTokenStr, from, to);
} catch (const exception &x) {
	cerr << "exception "  << x.what() << endl;
	return 1;
}
