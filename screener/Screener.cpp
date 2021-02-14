#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/program_options.hpp>
#include <date/date.h>
#include <util/log.h>

#include "DateDaysOps.h"
#include "SyncQuotes.h"
#include "Screen.h"
#include "ShowRet13612.h"

using namespace std;
using namespace tiingo;
using namespace screener;
using namespace date;

int main(int ac, char** av) try {
	const char
		*argHelp = "help",
		*argSync = "sync",
		*argScreen = "screen",
		*argRet13612 = "ret13612",
		*argTickers = "tickers",
		*argAuthToken = "auth-token",
		*argEndDate = "end-date",
		*argAcclelRate = "acceleration-rate",
		*argQuotesDir = "quotes-dir";

	Date to = floor<days>(chrono::system_clock::now());
	stringstream toStr;
	toStr << to;

	ScreenParams screenParams;
	string quotesDir;
	string tickersFile;
	screenParams.accelerationRate = GoldenRatioHi;

	namespace po = boost::program_options;
	po::options_description description("Screener");
	description.add_options()
		(argHelp, "Help message")
		(argSync, "Sync quotes with Tiingo")
		(argScreen, "Run screener")
		(argRet13612, "Show ret13612")
		(argQuotesDir, po::value<string>(&quotesDir)->required(), "Directory with Tiingo cache")
		(argTickers, po::value<string>(&tickersFile)->required(), "Ticker list file")
		(argEndDate, po::value<string>(), "End date")
		(argAcclelRate, po::value<double>(&screenParams.accelerationRate)->
			default_value(screenParams.accelerationRate), "Acceleration rate")
		(argAuthToken, po::value<string>(), "Tiingo auth token file")
		;

	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av).options(description).run(),
		vm
	);

	if (vm.count(argHelp) > 0) {
		LOG(description);
		return 1;
	}
	if (vm.count(argSync) + vm.count(argScreen) + vm.count(argRet13612) > 1) {
		LOG(
			"Only one of"
			<< " --" << argSync
			<< " --" << argScreen
			<< " --" << argRet13612
			<< " can be specified" << endl
			<< description
		);
		return 1;
	}

	po::notify(vm);

	if (vm.count(argEndDate)) {
		to = stringToDate(vm[argEndDate].as<string>());
		screenParams.toDate = to;
	}
	Date from = to - years{1} - days{Period * NumPeriods + ExtraDays};

	const auto getArg = [&](const char *argName) {
		if (vm.count(argName) != 1)
			ERROR(runtime_error, "missing or duplicate argument --" << argName);
		return vm[argName];
	};

	const auto getStream = [](const char *fileName) {
		ifstream stream(fileName);
		if (!stream)
			ERROR(runtime_error, "cannot open file: " << fileName);
		return stream;
	};

	auto tickers = getStream(tickersFile.c_str());

	if (vm.count(argSync)) {
		vector<string> authTokens;
		auto stream = getStream(getArg(argAuthToken).as<string>().c_str());
		copy(istream_iterator<string>(stream), istream_iterator<string>(), back_inserter(authTokens));
		syncQuotes(tickers, quotesDir, authTokens, from, to);
	} else if (vm.count(argScreen))
		screen(tickers, quotesDir, screenParams);
	else if (vm.count(argRet13612))
		showRet13612(tickers, quotesDir);

} catch (const exception &x) {
	LOG("error: "  << x.what());
	return 1;
}
