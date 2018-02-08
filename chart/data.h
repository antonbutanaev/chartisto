#ifndef CHART_DATA_H_
#define CHART_DATA_H_

#include <limits>
#include <cstddef>
#include <chrono>
#include <memory>
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

    virtual size_t numBars() const = 0;
};

using PBars = std::shared_ptr<Bars>;

class Points {
public:
    virtual ~Points() = default;

    virtual std::string title() const = 0;
    virtual Time time(size_t) const = 0;
    virtual Price close(size_t) const = 0;

    virtual size_t numPoints() const = 0;
};

using PPoints = std::shared_ptr<Points>;

class Bars2Points: public Points {
public:
	using Convert = std::function<Price(const Bars&, size_t)>;
	Bars2Points(
		const PBars &bars,
		const Convert &convert = [](const Bars &bars, size_t n) {return bars.close(n);}
	) : bars_(bars), convert_(convert)
	{}

    std::string title() const override {return bars_->title();}
    Time time(size_t n) const override {return bars_->time(n);}
    Price close(size_t n) const override {return convert_(*bars_, n);}

    size_t numPoints() const override {return bars_->numBars();}

private:
    PBars bars_;
    Convert convert_;
};

}
}

#endif
