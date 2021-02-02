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
using namespace screener;
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

	if (vm.count(argHelp) > 0) {
		cerr << description << endl;
		return 1;
	}
	if (vm.count(argSync) + vm.count(argScreen) > 1) {
		cerr
			<< "Only one of --" << argSync << " --" << "argScreen " << " can be specified" << endl
			<< description << endl;
		return 1;
	}

	po::notify(vm);

	const auto getArg = [&](const char *argName) {
		if (vm.count(argName) != 1)
			throw runtime_error(string("missing or duplicate argument --") + argName);
		return vm[argName];
	};

	const auto getStream = [](const char *fileName) {
		ifstream stream(fileName);
		if (!stream)
			throw runtime_error(string("cannot open file: ") + fileName);
		return stream;
	};

	auto tickers = getStream(tickersFile.c_str());

	if (vm.count(argSync)) {
		vector<string> authTokens;
		auto stream = getStream(getArg(argAuthToken).as<string>().c_str());
		copy(istream_iterator<string>(stream), istream_iterator<string>(), back_inserter(authTokens));
		syncQuotes(tickers, quotesDir, authTokens, from, to);
	} else if (vm.count(argScreen))
		screen(tickers, quotesDir);

} catch (const exception &x) {
	cerr << "error: "  << x.what() << endl;
	return 1;
}
