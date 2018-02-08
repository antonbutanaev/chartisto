#include <iostream>
#include <regex>
#include <date.h>
#include <chart/chart.h>
#include "quotesparser.h"

using namespace std;
using namespace std::chrono;
using namespace date;
using namespace chart;

namespace robotrade {

struct Bar {
	Time time;
	Price open, close, high, low;
	Volume volume;
};

struct Bars::Impl {
	vector<Bar> bars_;
	string title;
};

Bars::Bars() : i_(new Impl) {}

Bars::~Bars() {}

string Bars::title() const {return i_->title;}

Time Bars::time(size_t n) const {return i_->bars_[n].time;}

Price Bars::open(size_t n) const {return i_->bars_[n].open;}

Price Bars::close(size_t n) const {return i_->bars_[n].close;}

Price Bars::high(size_t n) const {return i_->bars_[n].high;}

Price Bars::low(size_t n) const {return i_->bars_[n].low;}

Volume Bars::volume(size_t n) const {return i_->bars_[n].volume;}

size_t Bars::numBars() const {return i_->bars_.size();}

data::PBars parse(std::istream &is) {
	auto result = std::make_shared<Bars>();

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

            if (result->i_->title.empty())
            	result->i_->title = match[Ticker];

            result->i_->bars_.push_back({
                dateTime,
                stod(match[Open]),
                stod(match[Close]),
                stod(match[High]),
                stod(match[Low]),
                stod(match[Volume])
            });
        }
    }

    return result;
}

}
