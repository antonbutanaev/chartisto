#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/program_options.hpp>
#include <date/date.h>
#include "SyncQuotes.h"
#include "Screen.h"

using namespace std;
using namespace tiingo;
using namespace date;

int main(int ac, char** av) try {
	const char
		*argHelp = "help",
		*argSync = "sync",
		*argScreen = "screen",
		*argTickers = "tickers",
		*argAuthToken = "auth-token",
		*argFromDate = "from-date",
		*argQuotesDir = "quotes-dir";

	Date to = floor<days>(chrono::system_clock::now());
	Date from = to - years{2};

	stringstream fromStr;
	fromStr << from;

	namespace po = boost::program_options;

	string quotesDir;
	string tickersFile;

	po::options_description description("Screener");
	description.add_options()
		(argHelp, "Help message")
		(argSync, "Sync quotes with Tiingo")
		(argScreen, "Run screener")
		(argQuotesDir, po::value<string>(&quotesDir)->required(), "Directory with Tiingo cache")
		(argTickers, po::value<string>(&tickersFile)->required(), "Ticker list file")
		(argFromDate, po::value<string>()->default_value(fromStr.str()), "From date")
		(argAuthToken, po::value<string>(), "Tiingo auth token file")
		;

	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av).options(description).run(),
		vm
	);

	if (vm.count(argHelp) > 0 || vm.count(argHelp) + vm.count(argSync) + vm.count(argScreen) > 1) {
		cerr
			<< "Hardware concurrency: " << thread::hardware_concurrency() << endl
			<< description << endl;
		return 1;
	}

	po::notify(vm);

	ifstream tickers(tickersFile);
	if (!tickers) {
		cerr << "Could not open tickers file " << vm[argTickers].as<string>() << endl;
		return 1;
	}

	if (vm.count(argSync)) {
		if (vm.count(argAuthToken) != 1)
		{
			cerr << "Need --" << argAuthToken << endl;
			return 1;
		}

		ifstream authToken(vm[argAuthToken].as<string>());
		if (!tickers) {
			cerr << "Could not open auth token file " << vm[argAuthToken].as<string>() << endl;
			return 1;
		}

		string authTokenStr;
		authToken >> authTokenStr;

		syncQuotes(tickers, quotesDir, authTokenStr, from, to);
	} else if (vm.count(argScreen))
		screen(tickers, quotesDir);

} catch (const exception &x) {
	cerr << "exception "  << x.what() << endl;
	return 1;
}
