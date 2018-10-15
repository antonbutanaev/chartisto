#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <atomic>
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
using namespace chart;
using namespace std;

void updateQuotes(const string &quoteUpdatesFile, const vector<string> &quoteFiles) {
	struct Quotes {
		string quoteFile;
		data::PBars bars;
	};

	map<string, Quotes> quotesByTicker;
	for (const auto &quoteFile: quoteFiles) {
		Quotes quotes;
		quotes.quoteFile = quoteFile;
		ifstream ifs(quoteFile.c_str());
		quotes.bars = robotrade::parse(ifs);
		quotesByTicker[quotes.bars->title(0)] = move(quotes);
	}

	ifstream ifs(quoteUpdatesFile.c_str());
	auto updateBars = robotrade::parse(ifs);

	for (size_t barNum = 0; barNum < updateBars->num(); ++barNum) {
		const auto quotesIt = quotesByTicker.find(updateBars->title(barNum));
		if (quotesIt == quotesByTicker.end()) {
			cerr << "Update quotes title " << updateBars->title(barNum) << " not found" << endl;
			continue;
		}

		const auto &quotes = quotesIt->second;
		ofstream ofs(quotes.quoteFile.c_str(), ios::app);

		cerr << "Update quotes title " << updateBars->title(barNum) << " FOUND" << endl;

		if (updateBars->time(barNum) > quotes.bars->time(quotes.bars->num()-1)) {
			cerr
				<< "Adding to " << quotes.quoteFile
				<< " ending with " << quotes.bars->time(quotes.bars->num()-1)
				<< " quote for " << updateBars->time(barNum)
				<< endl;

			const auto day = chrono::time_point_cast<date::days>(updateBars->time(barNum));
			const auto ymd = year_month_day(day);
			ofs
				<< updateBars->title(barNum) << ","
				<< "D,"
				<< setw(4) << setfill('0') << static_cast<int>(ymd.year())
				<< setw(2) << setfill('0') << static_cast<unsigned>(ymd.month())
				<< setw(2) << setfill('0') << static_cast<unsigned>(ymd.day()) << ","
				<< "000000,"
				<< updateBars->open(barNum) << ","
				<< updateBars->high(barNum) << ","
				<< updateBars->low(barNum) << ","
				<< updateBars->close(barNum) << ",0"
				<< endl;
		}
	}
}

void processLevels(const string &configJson, const vector<string> &quoteFiles) {
	atomic<unsigned> fileNum = 0;
	const auto runLevels = [&]{
		for (;;) {
			unsigned localFileNum = fileNum++;
			if (localFileNum >= quoteFiles.size())
				break;
			ifstream ifs(quoteFiles[localFileNum].c_str());
			if (!ifs)
				throw runtime_error("Could not open file: " + quoteFiles[localFileNum]);

			auto resultFile = quoteFiles[localFileNum];
			const auto slash = resultFile.find_last_of('/');
			if (slash != string::npos)
				resultFile = resultFile.substr(slash + 1);
			Levels(configJson, resultFile + ".result").process(robotrade::parse(ifs));
		};
	};

	vector<thread> threads;
	const auto nThreads = std::thread::hardware_concurrency();
	for (unsigned threadNum = 1; threadNum < nThreads; ++threadNum)
		threads.push_back(thread(runLevels));
	runLevels();
	for (auto &thread: threads)
		thread.join();
}

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argQuotes = "quotes",
		*argUpdateQuotes = "update-quotes",
		*argLevelsJson = "levels",
		*argLog = "log";

	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add(argQuotes, -1);
	po::options_description description("Allowed options");
	description.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argUpdateQuotes, po::value<string>(), "file with quote updates")
		(argLevelsJson, po::value<string>()->default_value("levels.json"), "levels .json file")
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

	if (vm.count(argQuotes)) {
		if (vm.count(argUpdateQuotes)) {
			updateQuotes(vm[argUpdateQuotes].as<string>(), vm[argQuotes].as<vector<string>>());
		} else if (vm.count(argLevelsJson)) {
			processLevels(vm[argLevelsJson].as<string>(), vm[argQuotes].as<vector<string>>());
		} else
			throw runtime_error("What to do with quotes?");
	}
	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
