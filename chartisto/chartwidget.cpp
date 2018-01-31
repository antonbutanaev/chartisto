#include <QPainter>
#include <QResizeEvent>

#include <QDebug>

#include "chartwidget.h"

namespace {
const ChartWidget::Coord dragSensitivity = 3;
}

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
}

void ChartWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPalette pal = palette();

    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QBrush brush(Qt::black);
    QPen pen(brush, 1);

    painter.setPen(pen);

    QRect rect(0, 0, size_.width(), size_.height());
    painter.drawRect(rect);

    if (canvas_.numCharts() != 0) {
        Coord y = 0;
        for (size_t n = 0; n < canvas_.numCharts(); ++n) {
            painter.drawText(5, y + 15, QString::number(n)); // FIXME
            qDebug() << "chart " << " n " << n << " h " << canvas_.chart(n).h();
            y += canvas_.chart(n).h();
            if (n != canvas_.numCharts() - 1)
                painter.drawLine(QPoint{0, y}, QPoint{size_.width(), y});
        }
    }
}

void ChartWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    size_ = event->size();
    canvas_.setCanvasSize({size_.width(), size_.height()});
}

void ChartWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    const auto mouseY = event->y();
    const auto leftPressed = (event->buttons() & Qt::LeftButton);
    if (leftButtonPressed_ != leftPressed) {
        leftButtonPressed_ = leftPressed;
        if (leftPressed)
            leftButtonPressedAt_ = mouseY;
    }

    if (canvas_.numCharts() != 0) {
        Coord y = 0;
        for (Pos n = 0; n < canvas_.numCharts() - 1; ++n) {
            y += canvas_.chart(n).h();

            if (abs(mouseY - y) < dragSensitivity) {
                setCursor(Qt::SplitVCursor);
                chartNumOver_ = n;
                break;
            } else {
                if (chartResized_ == NoPos)
                    setCursor(Qt::ArrowCursor);
                else
                    setCursor(Qt::SplitVCursor);

                chartNumOver_ = NoPos;
            }
        }
    }
}

void ChartWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    const auto mouseY = event->y();
    const auto leftPressed = (event->buttons() & Qt::LeftButton);
    if (leftButtonPressed_ != leftPressed) {
        leftButtonPressed_ = leftPressed;
        if (leftPressed)
            leftButtonPressedAt_ = mouseY;
        else {
            const auto delta =  mouseY - leftButtonPressedAt_;
            qDebug() << "dragged " << delta;
            if (chartResized_ != NoPos) {
                auto &chart = canvas_.chart(static_cast<size_t>(chartResized_));
                chart.setH(chart.h() + delta);
                chartResized_ = NoPos;
                setCursor(Qt::ArrowCursor);
                update();
            }
        }
    }
}

void ChartWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    chartResized_ = chartNumOver_;
}

