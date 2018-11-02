#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <json/json.h>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include <robotrade/quotesparser.h>
#include <util/ThreadPool.h>

#include "Levels.h"
#include "triplescreen.h"

using namespace robotrade;
using namespace date;
using namespace chart;
using namespace std;

void processLevels(
	const string &configJson, unsigned daysToAnalyze, const vector<string> &quoteFiles,
	bool printSummary, bool printResultFiles, unsigned seed
) {
	struct Result {
		Levels::ProcessResult result;
		std::string resultFile;
	};

	util::ThreadPool threadPool;

	vector<future<Result>> resultFutures;
	resultFutures.reserve(quoteFiles.size());

	vector<Result> results;
	results.reserve(quoteFiles.size());

	for (const auto &quoteFile: quoteFiles)
		resultFutures.push_back(threadPool.exec([&, quoteFile]{
				ifstream ifs(quoteFile.c_str());
				if (!ifs)
					throw runtime_error("Could not open file: " + quoteFile);

				auto resultFile = quoteFile;
				const auto slash = resultFile.find_last_of('/');
				if (slash != string::npos)
					resultFile = resultFile.substr(slash + 1);
				resultFile +=  + ".result";

				return Result{
					Levels(configJson, daysToAnalyze, resultFile).process(robotrade::parse(ifs), seed),
					quoteFile
				};
			}
		));

	for (auto &resultFuture: resultFutures)
		results.push_back(resultFuture.get());

	if (printSummary) {
		sort(
			results.begin(), results.end(),
			[](const auto &a, const auto &b) {return a.result.finResult > b.result.finResult;}
		);

		cout << "#\tTitle\t\tProfits\tLosses\tFinRes" << endl;
		size_t num = 0;
		size_t numLosses = 0;
		size_t numProfits = 0;
		double finResult = 0;
		for (const auto &resultsIt: results) {
			const auto &result = resultsIt.result;
			finResult += result.finResult;
			numProfits += result.numProfits;
			numLosses += result.numLosses;
			cout
				<< ++num << ".\t"
				<< setw(15) << left << result.title << '\t'
				<< result.numProfits << '\t'
				<< result.numLosses << '\t'
				<< result.finResult
				<< endl;
		}
		cout
			<< endl
			<< '\t'
			<< setw(15) << left << "Total" << '\t'
			<< numProfits << '\t'
			<< numLosses << '\t'
			<< finResult
			<< endl;
	}

	if (printResultFiles) {
		for (const auto &resultsIt: results)
			if (resultsIt.result.numOrders != 0)
				cout << resultsIt.resultFile << endl;
	}
}

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argQuotes = "quotes",
		*argLevelsJson = "levels",
		*argLevelsDays = "levels-days",
		*argLevelsSummary = "levels-summary",
		*argLevelsResults = "levels-results",
		*argSeed = "seed",
		*argLog = "log";

	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add(argQuotes, -1);
	po::options_description description("Allowed options");
	description.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argLevelsJson, po::value<string>()->default_value("levels.json"), "levels .json file")
		(argLevelsDays, po::value<unsigned>()->default_value(0), "Days to analyze, 0 means all")
		(argSeed, po::value<unsigned>()->default_value(0), "Seed for random generator")
		(argLevelsSummary, "Print levels summary")
		(argLevelsResults, "Print levels result files")
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
		if (vm.count(argLevelsJson)) {
			processLevels(
				vm[argLevelsJson].as<string>(),
				vm[argLevelsDays].as<unsigned>(),
				vm[argQuotes].as<vector<string>>(),
				vm.count(argLevelsSummary) > 0,
				vm.count(argLevelsResults) > 0,
				vm[argSeed].as<unsigned>()
			);
		} else
			throw runtime_error("What to do with quotes?");
	}
	return 0;
} catch (const exception &x) {
	cerr << "main: exception: " << x.what() << endl;
	return 1;
}
