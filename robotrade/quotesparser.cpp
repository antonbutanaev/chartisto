#include <iostream>
#include <regex>
#include <date.h>
#include <chart/chart.h>
#include "quotesparser.h"

using namespace std;
using namespace std::chrono;
using namespace date;

namespace robotrade {

void parse(std::istream &is) {
    string line;
    getline(is, line);
    line.pop_back();

    if (line != "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>")
        throw runtime_error("Wrong header: " + line);

    cout << line << endl;

    // VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000
    const regex re(R"RE(^(\w+),\w+,(\d{4})(\d{2})(\d{2}),(\d{2})(\d{2})\d{2},([\d.]+),([\d.]+),([\d.]+),([\d.]+),(\d+))RE");

    enum {
    	Ticker=1,
    	Year,
    	Month,
    	Day,
    	Hours,
    	Minutes,
    	Open,
    	High,
    	Low,
    	Close,
    	Volume,
    };

    while (is) {
        getline(is, line);
        line.pop_back();

        cout << line << endl;
        smatch match;
        if (regex_search(line, match, re)) {

            const auto date =
                 sys_days{year{stoi(match[Year])} / stoi(match[Month]) / stoi(match[Day])} +
                 hours{stoi(match[Hours])} + minutes{stoi(match[Minutes])};

			chart::Time t = date;
			cout << "Time:" << t << endl;
        }
    }
}

}
