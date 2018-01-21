#include <iostream>
#include <chart/chart.h>

namespace chart {

Canvas::Canvas() {
}

void Canvas::setCanvasSize(const Canvas::Size &) {

}

void Canvas::addChart(Chart &&chart) {
    charts_.emplace_back(std::move(chart));
}

void Canvas::setCursorPosition(const Canvas::Point &point) {
    cursorPosition_ = point;
}

size_t Canvas::numCharts() const {
    return charts_.size();
}

void Chart::addBars(Bars *) {

}

void Chart::addPoints(Point *) {

}

}
