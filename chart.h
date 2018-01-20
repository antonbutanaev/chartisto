#ifndef CHART_H
#define CHART_H

#include <limits>
#include <vector>
#include <memory>
#include <ctime>

namespace chart {

using Volume = double;
using Price = double;
const auto NoPrice = std::numeric_limits<Price>::lowest();
const auto NoTime = std::numeric_limits<time_t>::lowest();

class Candles {
public:
    virtual ~Candles() = default;

    virtual int size() const = 0;

    virtual time_t time(int) const = 0;
    virtual Price open(int) const = 0;
    virtual Price close(int) const = 0;
    virtual Price high(int) const = 0;
    virtual Price low(int) const = 0;

    virtual Volume volume(int) const = 0;
};

class Points {
public:
    virtual ~Points() = default;

    virtual int size(int) const = 0;

    virtual time_t time(int) const = 0;
    virtual Price close(int) const = 0;
};


class Chart {
public:
    virtual ~Chart() = default;

    virtual int numCandlesData() const = 0;
    virtual int numPointData() const = 0;

    virtual Candles &candles(int) const = 0;
    virtual Points &points(int) const = 0;

};

class Canvas {
public:
    struct Size {int x,y;};
    struct Point {int x,y;};

    Canvas();
    void setCanvasSize(const Size&);
    void addChart(Chart*);
    void setCursorPosition(const Point&);
    int numCharts() const;

private:
    std::vector<std::unique_ptr<Chart>> charts_;
    Point cursorPosition_;

};

}

#endif // PAINTER_H
