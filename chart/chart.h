#ifndef CHART_H
#define CHART_H

#include <limits>
#include <memory>
#include <vector>
#include <cstddef>
#include <ctime>
#include <chrono>

namespace chart {

using Volume = double;
using Price = double;
using Time = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
constexpr auto NoPrice = std::numeric_limits<Price>::max();
constexpr auto NoTime = Time::max();

namespace data {

class Bar {
public:
    virtual ~Bar() = default;

    virtual Time time() const = 0;
    virtual Price open() const = 0;
    virtual Price close() const = 0;
    virtual Price high() const = 0;
    virtual Price low() const = 0;

    virtual Volume volume() const = 0;
};

class BarImpl: public Bar {
public:

	BarImpl(Time time, Price open, Price close, Price high, Price low, Volume volume) :
		time_(time), open_(open), close_(close), high_(high), low_(low), volume_(volume) {}

    Time time() const override {return time_;}
    Price open() const override {return open_;}
    Price close() const override {return close_;}
    Price high() const override {return high_;}
    Price low() const override {return low_;}

    Volume volume() const override {return volume_;}

private:
    Time time_;
	Price open_, close_, high_, low_;
	Volume volume_;
};

class Bars {
public:
    virtual ~Bars() = default;

    virtual size_t numBars() const = 0;
    virtual Bar &bar(size_t) const = 0;
};

class Point {
public:
    virtual ~Point() = default;

    virtual Time time(int) const = 0;
    virtual Price close(int) const = 0;
};

class Points {
public:
    virtual ~Points() = default;

    virtual size_t numPoints() const = 0;
    virtual Point &point(size_t) const = 0;
};

}

class Canvas;

class Chart {
    friend class Canvas;
public:
    class Bar {
        const data::Bar &bar() const;
        int high() const;
        int low() const;
        int open() const;
        int close() const;
        int volume() const;

        int x() const;
        int w() const;
    };

    class Point {
        const data::Point &bar() const;
        int close() const;

        int x() const;
        int w() const;
    };

    void addBars(std::unique_ptr<data::Bars>&&);
    void addPoints(std::unique_ptr<data::Points>&&);

    size_t numBars() const;
    size_t numPoints() const;

    const Bar &bar(size_t) const;
    const Point &point(size_t) const;

    int h() const {return h_;}
    void setH(int h) {h_ = h;}

private:

    int h_ = 0;
};

class Canvas {
public:
    struct Size {int w,h;};
    struct Point {int x,y;};

    Canvas();
    void setCanvasSize(const Size&);
    void addChart(Chart&&);
    void setCursorPosition(const Point&);
    size_t numCharts() const;
    Chart &chart(size_t n);

private:
    std::vector<Chart> charts_;
    Point cursorPosition_;
    Size size_;
};

}

#endif
