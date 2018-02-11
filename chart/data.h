#ifndef CHART_DATA_H_
#define CHART_DATA_H_

#include <limits>
#include <cstddef>
#include <chrono>
#include <memory>
#include <utility>
#include <functional>

namespace chart {

using Volume = double;
using Price = double;
using Time = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
constexpr auto NoPrice = std::numeric_limits<Price>::max();
constexpr auto NoTime = Time::max();

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
    CreatePoints(Source source, FClose fClose) :
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
        std::forward<Source>(source), std::forward<FClose>(fClose)
    );
}

}
}

#endif
