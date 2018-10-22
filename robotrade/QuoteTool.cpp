#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <util/Stream.h>
#include <robotrade/quotesparser.h>

using namespace std;
using namespace date;

void splitQuotes(const string &quotesFile) {
	cout << "Parsing...\n";
	util::Stream<ifstream> ifs(quotesFile.c_str());
	const auto multiBars = robotrade::parse(ifs);
	cout << "Parsed\n";
	map<string, util::Stream<ofstream>> outFiles;
	for (size_t barNum = 0; barNum < multiBars->num(); ++barNum) {
		auto it = outFiles.find(multiBars->title(barNum));
		if (it == outFiles.end()) {
			it = outFiles.emplace(
				multiBars->title(barNum),
				util::Stream<ofstream>(multiBars->title(barNum) + ".txt")
			).first;
		}
		ofstream &ofs = it->second;
		ofs.precision(10);
		const auto day = chrono::time_point_cast<date::days>(multiBars->time(barNum));
		const auto ymd = year_month_day(day);
		ofs
			<< multiBars->title(barNum) << ","
			<< "D,"
			<< setw(4) << setfill('0') << static_cast<int>(ymd.year())
			<< setw(2) << setfill('0') << static_cast<unsigned>(ymd.month())
			<< setw(2) << setfill('0') << static_cast<unsigned>(ymd.day()) << ","
			<< "000000,"
			<< multiBars->open(barNum) << ","
			<< multiBars->high(barNum) << ","
			<< multiBars->low(barNum) << ","
			<< multiBars->close(barNum) << ","
			<< multiBars->volume(barNum) << ",0"
			<< endl;
	}
}

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argQuotes = "quotes",
		*argSplitQuotes = "split-quotes",
		*argLog = "log";

	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add(argQuotes, -1);
	po::options_description description("Allowed options");
	description.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argSplitQuotes, po::value<string>(), "split one multiticker quote file into separate files")
		(argLog, po::value<string>()->default_value("robotrade_log.conf"), "log .conf file");


	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av)
			.options(description)
			.positional(positional)
			.run(),
		vm
	);
	po::notify(vm);

	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure(vm["log"].as<string>());

	auto logger = log4cplus::Logger::getInstance("main");
	LOG4CPLUS_DEBUG(logger, "start");

	if (vm.count(argHelp)) {
		cout << description << endl;
		return 1;
	}

	if (vm.count(argSplitQuotes)) {
		splitQuotes(vm[argSplitQuotes].as<string>());
	}

	return 0;
} catch (const exception &x) {
	cerr << "main: exception: " << x.what() << endl;
	return 1;
}
