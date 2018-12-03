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

#include <robotrade/quotesParser.h>
#include <util/async.h>
#include <util/hasher.h>

#include <robotrade/levels.h>
#include <robotrade/tripleScreen.h>
#include <robotrade/emaCross.h>

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

	util::Async async;

	auto results = async.execTasks(
		util::funcIterator(quoteFiles),
		[&](const string &quoteFile) {
			return [&, quoteFile] {
				ifstream ifs(quoteFile.c_str());
				if (!ifs)
					throw runtime_error("Could not open file: " + quoteFile);

				auto resultFile = quoteFile;
				const auto slash = resultFile.find_last_of('/');
				if (slash != string::npos)
					resultFile = resultFile.substr(slash + 1);
				resultFile += ".result";

				return Result{
					Levels(configJson, daysToAnalyze, resultFile).process(robotrade::parse(ifs), seed),
					quoteFile
				};
			};
		}
	);

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
		*argTodayQuotes = "today-quotes",
		*argDays = "days",
		*argSummary = "summary",
		*argOrders = "orders",
		*argVerbose = "verbose",
		*argSeed = "seed",
		*argLevelsJson = "levels",
		*argEMACrossJson = "ema-cross";

	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add(argQuotes, -1);
	po::options_description description("Allowed options");
	description.add_options()
		(argHelp, "produce help message")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argTodayQuotes, po::value<string>(), "dir with today quotes")
		(argLevelsJson, po::value<string>(), "levels .json file")
		(argEMACrossJson, po::value<string>(), "EMA cross .json file")
		(argVerbose, po::value<unsigned>()->default_value(0), "Verbose")
		(argDays, po::value<unsigned>()->default_value(0), "Days to analyze, 0 means all")
		(argSeed, po::value<unsigned>()->default_value(0), "Seed for random generator")
		(argSummary, "Print summary")
		(argOrders, "Print orders");

	po::variables_map vm;
	po::store(
		po::command_line_parser(ac, av)
			.options(description)
			.positional(positional)
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

	if (vm.count(argQuotes)) {
		if (vm.count(argLevelsJson)) {
			processLevels(
				vm[argLevelsJson].as<string>(),
				vm[argDays].as<unsigned>(),
				vm[argQuotes].as<vector<string>>(),
				vm.count(argSummary) > 0,
				vm.count(argOrders) > 0,
				vm[argSeed].as<unsigned>()
			);
		} if (vm.count(argEMACrossJson)) {
			EMACross(
				vm[argEMACrossJson].as<string>(),
				vm[argVerbose].as<unsigned>()
			).process(
				vm.count(argSummary) > 0,
				vm.count(argOrders) > 0,
				vm[argDays].as<unsigned>(),
				vm[argQuotes].as<vector<string>>(),
				vm.count(argTodayQuotes) > 0? vm[argTodayQuotes].as<string>() : string(),
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
