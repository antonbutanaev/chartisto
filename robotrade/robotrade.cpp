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

	map<string, Quotes> quotesByTitle;
	for (const auto &quoteFile: quoteFiles) {
		Quotes quotes;
		quotes.quoteFile = quoteFile;
		ifstream ifs(quoteFile.c_str());
		if (!ifs)
			throw runtime_error("could not open " + quoteFile);
		quotes.bars = robotrade::parse(ifs);
		quotesByTitle[quotes.bars->title(0)] = move(quotes);
	}

	ifstream ifs(quoteUpdatesFile.c_str());
	if (!ifs)
		throw runtime_error("could not open " + quoteUpdatesFile);
	const auto updateBars = robotrade::parse(ifs);

	size_t added = 0;
	for (size_t barNum = 0; barNum < updateBars->num(); ++barNum) {
		const auto quotesIt = quotesByTitle.find(updateBars->title(barNum));
		if (quotesIt == quotesByTitle.end()) {
			cerr << "Update quotes, title " << updateBars->title(barNum) << " not found" << endl;
			continue;
		}

		const auto &quotes = quotesIt->second;
		ofstream ofs(quotes.quoteFile.c_str(), ios::app);
		if (!ofs)
			throw runtime_error("could not open " + quoteUpdatesFile);

		ofs.precision(10);
		auto lastBarNum = quotes.bars->num() - 1;
		if (updateBars->time(barNum) > quotes.bars->time(lastBarNum)) {
			++added;
			const auto time = updateBars->time(barNum);
			const auto day = chrono::time_point_cast<date::days>(time);
			const auto ymd = year_month_day(day);
			const auto tod = date::make_time(time - day);
			ofs
				<< updateBars->title(barNum) << ","
				<< "D,"
				<< setfill('0')
				<< setw(4) << static_cast<int>(ymd.year())
				<< setw(2) << static_cast<unsigned>(ymd.month())
				<< setw(2) << static_cast<unsigned>(ymd.day()) << ","
				<< setw(2) << tod.hours().count()
				<< setw(2) << tod.minutes().count()
				<< "00,"
				<< updateBars->open(barNum) << ","
				<< updateBars->high(barNum) << ","
				<< updateBars->low(barNum) << ","
				<< updateBars->close(barNum) << ","
				<< fixed << setprecision(0) << updateBars->volume(barNum) << ",0"
				<< endl;
		}
	}
	cout << "Processed " << updateBars->num() << " update quotes, added " << added << endl;
}

void processLevels(
	const string &configJson, int daysToAnalyze, const vector<string> &quoteFiles,
	bool printSummary, bool printResultFiles, unsigned seed
) {
	atomic<unsigned> fileNum = 0;

	mutex resultsMutex;
	struct Result {
		Levels::ProcessResult result;
		std::string resultFile;
	};
	vector<Result> results;

	const auto runLevels = [&]{
		try {
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
				resultFile +=  + ".result";

				const auto result =
					Levels(configJson, daysToAnalyze, resultFile).process(robotrade::parse(ifs), seed);

				lock_guard l(resultsMutex);
				results.push_back({result, resultFile});
			};
		} catch (const exception &x) {
			cerr << "Levels: exception: " << x.what() << endl;
		}
	};

	vector<thread> threads;
	const auto nThreads = std::thread::hardware_concurrency();
	for (unsigned threadNum = 1; threadNum < nThreads; ++threadNum)
		threads.push_back(thread(runLevels));
	runLevels();
	for (auto &thread: threads)
		thread.join();

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
		*argUpdateQuotes = "update-quotes",
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
		(argUpdateQuotes, po::value<string>(), "file with quote updates")
		(argLevelsJson, po::value<string>()->default_value("levels.json"), "levels .json file")
		(argLevelsDays, po::value<int>()->default_value(0), "Days to analyze, 0 means all")
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
		if (vm.count(argUpdateQuotes)) {
			updateQuotes(vm[argUpdateQuotes].as<string>(), vm[argQuotes].as<vector<string>>());
		} else if (vm.count(argLevelsJson)) {
			processLevels(
				vm[argLevelsJson].as<string>(),
				vm[argLevelsDays].as<int>(),
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
