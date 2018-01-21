#include <iostream>
#include <chart/chart.h>

namespace chart {

Canvas::Canvas() {
}

void Canvas::setCanvasSize(const Canvas::Size &size) {
    size_ = size;
}

void Canvas::addChart(Chart &&chart) {
    charts_.emplace_back(std::move(chart));
    auto &addedChart = charts_.back();
    if (charts_.size() == 1) {
        addedChart.setH(size_.h);
        addedChart.setY(0);
    }
}

void Canvas::setCursorPosition(const Canvas::Point &point) {
    cursorPosition_ = point;
}

size_t Canvas::numCharts() const {
    return charts_.size();
}

void Chart::addBars(data::Bars *) {

}

void Chart::addPoints(data::Points *) {
}

}
