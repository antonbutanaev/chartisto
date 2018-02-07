#include <iostream>
#include <regex>
#include <date.h>
#include <chart/chart.h>
#include "quotesparser.h"

using namespace std;
using namespace std::chrono;
using namespace date;

namespace robotrade {

void parse(std::istream &is, const ParseCallback &cb) {
    string line;
    const auto chopLine = [&]{
      while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
          line.pop_back();
    };

    getline(is, line);
    chopLine();

    if (line != "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>")
        throw runtime_error("Wrong header: " + line);

    const regex re(R"RE(^(\w+),\w+,(\d{4})(\d{2})(\d{2}),(\d{2})(\d{2})\d{2},([\d.]+),([\d.]+),([\d.]+),([\d.]+),(\d+)$)RE");

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

    for(;;) {
        getline(is, line);
        if (!is)
        	break;
        chopLine();

        smatch match;
        if (regex_search(line, match, re)) {

            const auto dateTime =
                sys_days{year{stoi(match[Year])} / stoi(match[Month]) / stoi(match[Day])} +
                hours{stoi(match[Hours])} +
                minutes{stoi(match[Minutes])};

            chart::data::BarImpl bar{
                dateTime,
                stod(match[Open]),
                stod(match[Close]),
                stod(match[High]),
                stod(match[Low]),
                stod(match[Volume])
            };

            cb({match[Ticker], bar});
        }
    }
}

}
