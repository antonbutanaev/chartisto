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
    } else {
        const auto reduce = [] (auto h) {return h * 3 / 4;};
        addedChart.setH(size_.h - reduce(size_.h));
        addedChart.setY(size_.h - addedChart.h());

        int offset = 0;
        for (size_t i = 0; i < charts_.size() - 1; ++i) {
            auto &existedChart = charts_[i];
            existedChart.setY(existedChart.y() - offset);
            const auto oldH = existedChart.h();
            existedChart.setH(reduce(oldH));
            offset += oldH - existedChart.h();
        }
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
