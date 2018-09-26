#ifndef CHART_DATA_H_
#define CHART_DATA_H_

#include <limits>
#include <cstddef>
#include <chrono>
#include <memory>
#include <utility>
#include <fmt/ostream.h>
#include <date/date.h>

namespace chart {

using Volume = double;
using Price = double;
using Time = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
constexpr auto NoPrice = std::numeric_limits<Price>::max();
constexpr auto NoTime = Time::max();

inline std::ostream &operator<<(std::ostream &o, Time time) {
	const auto day = std::chrono::time_point_cast<date::days>(time);
	const auto ymd = date::year_month_day(day);
	auto tod = date::make_time(time - day);
	return o << ymd << ' ' << tod;
}

namespace data {

class Bars {
public:
    virtual ~Bars() = default;

    virtual std::string title() const = 0;
    virtual Time time(size_t) const = 0;
    virtual Price open(size_t) const = 0;
    virtual Price close(size_t) const = 0;
    virtual Price high(size_t) const = 0;
    virtual Price low(size_t) const = 0;
    virtual Volume volume(size_t) const = 0;

    virtual size_t num() const = 0;

    enum PriceType {Low,Open,Close,High};
    Price get(PriceType priceType, size_t num) {
    	switch (priceType) {
    	case Open: return open(num);
    	case Close: return close(num);
    	case High: return high(num);
    	case Low: return low(num);
    	default:
    		throw std::runtime_error("bad price type: " + std::to_string(static_cast<int>(priceType)));
    	}
    }
};

using PBars = std::shared_ptr<Bars>;

class Points {
public:
    virtual ~Points() = default;

    virtual std::string title() const = 0;
    virtual Time time(size_t) const = 0;
    virtual Price close(size_t) const = 0;

    virtual size_t num() const = 0;
};

using PPoints = std::shared_ptr<Points>;

template<class Source, class FClose>
class CreatePoints: public Points {
public:
    CreatePoints(Source &&source, FClose &&fClose) :
        source_(std::move(source)),
        fClose_(std::move(fClose)) {
    }

    std::string title() const override {return source_->title();}
    Time time(size_t n) const override {return source_->time(n);}
    Price close(size_t n) const override {return fClose_(n);}

    size_t num() const override {return source_->num();}

private:
    Source source_;
    FClose fClose_;
};

template<class Source, class FClose>
auto createPoints(Source source, FClose fClose) {
    return std::make_shared<CreatePoints<Source, FClose>>(
        std::move(source), std::move(fClose)
    );
}

}
}

#endif
