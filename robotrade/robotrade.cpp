#include <iostream>
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

using namespace std;

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
	for (unsigned i = 1; i < nThreads; ++i)
		threads.push_back(thread(runLevels));
	runLevels();
	for (auto &thread: threads)
		thread.join();
}

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argQuotes = "quotes",
		*argLevelsJson = "levels",
		*argLog = "log";

	namespace po = boost::program_options;

	po::positional_options_description p;
	p.add(argQuotes, -1);

	po::options_description desc("Allowed options");
	desc.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argLevelsJson, po::value<string>()->default_value("levels.json"), "levels .json file")
		(argLog, po::value<string>()->default_value("robotrade_log.conf"), "log .conf file");

	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av)
			.options(desc)
			.positional(p)
			.run(),
		vm
	);
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
		if (vm.count(argLevelsJson)) {
			processLevels(vm[argLevelsJson].as<string>(), vm[argQuotes].as<vector<string>>());
		} else
			throw runtime_error("What to do with quotes?");
	}
	return 0;
} catch (const std::exception &x) {
	cerr << x.what() << endl;
	return 1;
}
