#include <iostream>
#include <chart/chart.h>

namespace chart {

Canvas::Canvas() {
}

void Canvas::setCanvasSize(const Canvas::Size &) {

}

void Canvas::addChart(Chart *chart) {
    charts_.emplace_back(chart);
}

void Canvas::setCursorPosition(const Canvas::Point &point) {
    cursorPosition_ = point;
}

int Canvas::numCharts() const {
    return charts_.size();
}

}
