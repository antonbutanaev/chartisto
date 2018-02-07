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

class Points {
public:
    virtual ~Points() = default;

    virtual std::string title() const = 0;
    virtual Time time(size_t) const = 0;
    virtual Price close(size_t) const = 0;

    virtual size_t numPoints() const = 0;
};

}

class Canvas;

class Chart {
    friend class Canvas;
public:
    void addBars(std::unique_ptr<data::Bars>&&);
    void addPoints(std::unique_ptr<data::Points>&&);

    size_t numBars() const;
    size_t numPoints() const;

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
