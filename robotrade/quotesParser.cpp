#include <iostream>
#include <vector>
#include <date/date.h>
#include <chart/data.h>
#include <robotrade/quotesParser.h>

using namespace std;
using namespace std::chrono;
using namespace date;
using namespace chart;

namespace robotrade {
namespace {

const char *header1 = "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>";
const char *header2 = "<TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>,<OPENINT>";

class Bars: public data::Bars {
public:
    Bars(std::istream &is) {
        int lineNum = 1;
        const auto error = [&] (const string &message) {
        	throw runtime_error(message + " at line " + to_string(lineNum));
        };

        string line;
        if(!getline(is, line))
        	error("no header");

        const auto fmt1 = line == header1;
        const auto fmt2 = line == header2;

        if (!fmt1 && !fmt2)
            throw runtime_error("wrong header: " + line);

        const auto check = [&] (char delim, int item) {
        	char c;
        	is.get(c);
        	if (c != delim)
        		error("expected delim after item " + to_string(item) + " got " + c);
        };

        for(;;) {
        	++lineNum;
        	int item = 0;
        	Bar bar;
            if(!getline(is, bar.title, ','))
            	break;

            char per;
            is >> per;
            check(',', ++item);

            int date;
            is >> date;
            check(',', ++item);

            int time;
            is >> time;
            check(',', ++item);

            const auto d = date % 100;
            date /= 100;
            const auto m = date % 100;
            const auto y = date / 100;

            time /= 100;
            const auto M = time % 100;
            const auto H = time / 100;

			bar.time = sys_days{year{y}/m/d} + hours{H} + minutes{M};

			is >> bar.open;
            check(',', ++item);

			is >> bar.high;
            check(',', ++item);

            is >> bar.low;
            check(',', ++item);

            is >> bar.close;
            check(',', ++item);

            is >> bar.volume;

            if (fmt2) {
            	check(',', ++item);
            	int openInt;
				is >> openInt;
            }

            check('\n', ++item);

			bars_.push_back(move(bar));
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
