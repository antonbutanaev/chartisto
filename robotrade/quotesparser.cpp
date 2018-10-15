#include <iostream>
#include <regex>
#include <date/date.h>
#include <chart/data.h>
#include <robotrade/quotesparser.h>

using namespace std;
using namespace std::chrono;
using namespace date;
using namespace chart;

namespace robotrade {
namespace {

class Bars: public data::Bars {
public:
    Bars(std::istream &is) {
        string line;
        const auto chopLine = [&]{
          while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
              line.pop_back();
        };

        getline(is, line);
        chopLine();

        if (
        	line != "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>" &&
        	line != "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>,<OPENINT>"
        )
            throw runtime_error("Wrong header: " + line);

        const regex re(R"RE(^([^,]+),\w+,(\d{4})(\d{2})(\d{2}),(\d{2})(\d{2})\d{2},([\d.]+),([\d.]+),([\d.]+),([\d.]+),(\d+)(,\d+)?$)RE");

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

                bars_.push_back({
                    dateTime,
                    stod(match[Open]),
                    stod(match[Close]),
                    stod(match[High]),
                    stod(match[Low]),
                    stod(match[Volume]),
                    match[Ticker],
                });
            }
        }
    }

private:
    std::string title(size_t n) const override {return bars_[n].title;}
    Time time(size_t n) const override {return bars_[n].time;}
    Price open(size_t n) const override {return bars_[n].open;}
    Price close(size_t n) const override {return bars_[n].close;}
    Price high(size_t n) const override {return bars_[n].high;}
    Price low(size_t n) const override {return bars_[n].low;}
    Volume volume(size_t n) const override {return bars_[n].volume;}

    size_t num() const override {return bars_.size();}

    struct Bar {
        Time time;
        Price open, close, high, low;
        Volume volume;
        string title;
    };
    vector<Bar> bars_;
};

}

data::PBars parse(std::istream &is) {
    return make_shared<Bars>(is);
}

}
