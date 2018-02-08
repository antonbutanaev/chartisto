#ifndef CHART_H
#define CHART_H

#include <vector>
#include <cstddef>
#include <chart/data.h>

namespace chart {

class Canvas;

class Chart {
    friend class Canvas;
public:
    void addBars(const data::PBars&);
    void addPoints(const data::PPoints&);

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
