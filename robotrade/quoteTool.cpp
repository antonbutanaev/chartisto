#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <json/json.h>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <util/stream.h>
#include <robotrade/quotesParser.h>
#include <robotrade/todayQuotes.h>

using namespace std;
using namespace date;
using namespace chart;

void splitQuotes(const string &quotesFile, string suffix) {
	if (!suffix.empty())
		suffix = '_' + suffix;
	cout << "Parsing...\n";
	util::Stream<ifstream> ifs(quotesFile.c_str());
	const auto multiBars = robotrade::parse(ifs);
	cout << "Parsed\n";
	unordered_map<string, util::Stream<ofstream>> outFiles;
	for (size_t barNum = 0; barNum < multiBars->num(); ++barNum) {
		bool insertHeader = false;
		auto it = outFiles.find(multiBars->title(barNum));
		if (it == outFiles.end()) {
			it = outFiles.emplace(
				multiBars->title(barNum),
				util::Stream<ofstream>(multiBars->title(barNum) + suffix + ".txt")
			).first;
			insertHeader = true;
		}
		ofstream &ofs = it->second;
		if (insertHeader)
			ofs << "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>,<OPENINT>" << endl;
		ofs.precision(10);
		const auto time = multiBars->time(barNum);
		const auto day = chrono::time_point_cast<date::days>(time);
		const auto ymd = year_month_day(day);
		const auto tod = date::make_time(time - day);
		ofs
			<< multiBars->title(barNum) << ","
			<< "D,"
			<< setfill('0')
			<< setw(4) << static_cast<int>(ymd.year())
			<< setw(2) << static_cast<unsigned>(ymd.month())
			<< setw(2) << static_cast<unsigned>(ymd.day()) << ","
			<< setw(2) << tod.hours().count()
			<< setw(2) << tod.minutes().count()
			<< "00,"
			<< multiBars->open(barNum) << ","
			<< multiBars->high(barNum) << ","
			<< multiBars->low(barNum) << ","
			<< multiBars->close(barNum) << ","
			<< multiBars->volume(barNum) << ",0"
			<< endl;
	}
}

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

int main(int ac, char *av[]) try {
	const char
		*argHelp = "help",
		*argSuffix = "suffix",
		*argQuotes = "quotes",
		*argTodayQuotes = "today-quotes",
		*argUpdateQuotes = "update-quotes",
		*argPrintQuotes = "print-quotes",
		*argTitleToTicker = "title-to-ticker",
		*argSplitQuotes = "split-quotes";

	namespace po = boost::program_options;

	po::positional_options_description positional;
	positional.add(argQuotes, -1);
	po::options_description description("Allowed options");
	description.add_options()
		(argHelp, "produce help message")
		(argPrintQuotes, "print quotes")
		(argQuotes, po::value<vector<string>>(), "file with quotes")
		(argTodayQuotes, po::value<string>(), "dir with today quotes")
		(argTitleToTicker, po::value<string>(), "title to ticker json")
		(argUpdateQuotes, po::value<string>(), "file with quote updates")
		(argSuffix, po::value<string>(), "add suffix to split files")
		(argSplitQuotes, po::value<string>(), "split one multiticker quote file into separate files");


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
		cout << description << endl;
		return 1;
	}

	if (vm.count(argSplitQuotes)) {
		splitQuotes(vm[argSplitQuotes].as<string>(), vm[argSuffix].as<string>());
	} else if (vm.count(argUpdateQuotes)) {
		const auto quotes = vm[argQuotes].as<vector<string>>();
		if (quotes.empty())
			throw runtime_error("No quotes to update");
		updateQuotes(vm[argUpdateQuotes].as<string>(), quotes);
	} else if (vm.count(argPrintQuotes)) {
		for (const auto &quotesFile: vm[argQuotes].as<vector<string>>()) {
			auto jsonStream = util::check<ifstream>(vm[argTitleToTicker].as<string>());
			Json::Value titleToTicker;
			jsonStream >> titleToTicker;
			const auto bars = robotrade::parseWithToday(
				quotesFile,
				vm[argTodayQuotes].as<string>(),
				titleToTicker
			);
			cout << "title\ttime\topen\thigh\tlow\tclose\tvolume" << endl;
			for (size_t barNum = 0; barNum < bars->num(); ++barNum)
				cout
					<< bars->title(barNum) << '\t'
					<< bars->time(barNum) << '\t'
					<< bars->open(barNum) << '\t'
					<< bars->high(barNum) << '\t'
					<< bars->low(barNum) << '\t'
					<< bars->close(barNum) << '\t'
					<< bars->volume(barNum) << endl;
		}
	}
	return 0;
} catch (const exception &x) {
	cerr << "main: exception: " << x.what() << endl;
	return 1;
}
