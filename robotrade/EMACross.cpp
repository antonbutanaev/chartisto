#include <iostream>
#include <fstream>
#include <robotrade/EMACross.h>
#include <robotrade/quotesParser.h>


using namespace std;
using namespace util;

namespace robotrade {

EMACross::EMACross() {
}

void EMACross::process(FuncItreator<const string> quoteFiles) {
	const auto results = async_.execTacks(
		quoteFiles,
		[&] (const string &quoteFile) {
			return [&, quoteFile] {
				ifstream ifs(quoteFile.c_str());
				if (!ifs)
					throw runtime_error("Could not open file: " + quoteFile);
				return process(robotrade::parse(ifs));
			};
		}
	);

	cout << "Results:" << endl;
	for (const auto &result: results)
		cout << result.title << ' ' << result.numQuotes << endl;
}

EMACross::Result EMACross::process(chart::data::PBars bars) {
	return {
		bars->title(0),
		bars->num()
	};
}

}
