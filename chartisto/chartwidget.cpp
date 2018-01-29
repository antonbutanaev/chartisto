#include <QPainter>
#include <QResizeEvent>

#include <QDebug>

#include "chartwidget.h"

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
}

void ChartWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    qDebug() << "paint";

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
        int y = 0;
        for (size_t n = 0; n < canvas_.numCharts(); ++n) {
            painter.drawText(5, y + 15, QString::number(n));
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
    const auto my = event->y();
    const auto leftPressed = (event->buttons() & Qt::LeftButton);
    if (leftPressed_ != leftPressed) {
        leftPressed_ = leftPressed;
        if (leftPressed)
            leftPressedAt_ = my;
        else {
            qDebug() << "dragged " << my - leftPressedAt_;
        }
    }


    if (canvas_.numCharts() != 0) {
        int y = 0;
        for (size_t n = 0; n < canvas_.numCharts() - 1; ++n) {
            y += canvas_.chart(n).h();

            if (abs(my - y) < 3) {
                setCursor(Qt::SplitVCursor);
                chartNumOver_ = static_cast<int>(n);
                break;
            } else {
                if (chartResized_ == -1)
                    setCursor(Qt::ArrowCursor);
                else
                    setCursor(Qt::SplitVCursor);

                chartNumOver_ = -1;

            }
        }
    }

}

void ChartWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    const auto my = event->y();
    const auto leftPressed = (event->buttons() & Qt::LeftButton);
    if (leftPressed_ != leftPressed) {
        leftPressed_ = leftPressed;
        if (leftPressed)
            leftPressedAt_ = my;
        else {
            const auto delta =  my - leftPressedAt_;
            qDebug() << "dragged " << delta;
            if (chartResized_ != -1 ) {
                qDebug() << "resizing chart " << chartResized_ << " over " << delta;

                auto &chart = canvas_.chart(static_cast<size_t>(chartResized_));

                chart.setH(chart.h() + delta);


                chartResized_ = -1;
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

