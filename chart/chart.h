#ifndef CHART_H
#define CHART_H

#include <limits>
#include <vector>
#include <cstddef>
#include <ctime>

namespace chart {

using Volume = double;
using Price = double;
using Time = time_t;
const auto NoPrice = std::numeric_limits<Price>::lowest();
const auto NoTime = std::numeric_limits<Time>::lowest();

namespace data {

class Bar {
public:
    virtual ~Bar() = default;

    virtual time_t time(int) const = 0;
    virtual Price open(int) const = 0;
    virtual Price close(int) const = 0;
    virtual Price high(int) const = 0;
    virtual Price low(int) const = 0;

    virtual Volume volume(int) const = 0;
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

    virtual time_t time(int) const = 0;
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

    void addBars(data::Bars*);
    void addPoints(data::Points*);

    size_t numBars() const;
    size_t numPoints() const;

    const Bar &bar(size_t) const;
    const Point &point(size_t) const;

    int y() const {return y_;}
    int h() const {return h_;}
private:
    void setY(int y) {y_ = y;}
    void setH(int h) {h_ = h;}

    int y_ = 0;
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
    const Chart &chart(size_t n) const {return charts_[n];}

private:
    std::vector<Chart> charts_;
    Point cursorPosition_;
    Size size_;
};

}

#endif // PAINTER_H
