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

    cout << "line" << endl;

    string line;
    getline(is, line);
    line.pop_back();

    if (line != "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>")
        throw runtime_error("Wrong header: " + line);

    cout << line << endl;

    // VTBR,D,20130108,000000,0.0550000,0.0554000,0.0548300,0.0549200,23315530000
    const regex re(R"RE(^(\w+),D,(\d{4})(\d{2})(\d{2}),(\d{2})(\d{2})\d{2},([\d.]+),([\d.]+),([\d.]+),([\d.]+),(\d+))RE");

    int i=0;
    while (is) {
        getline(is, line);
        line.pop_back();

        if(++i > 10)
            break;

        cout << line << endl;
        smatch match;
        if (regex_search(line, match, re)) {

            const auto date =
                  sys_days{year{stoi(match[2])} / stoi(match[3]) / stoi(match[4])} +
    hours{stoi(match[5])} + minutes{stoi(match[6])};


    chart::Time t = date;

            cout << "MA:" << t << endl;
        }
    }
}


}
