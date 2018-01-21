#include <QPainter>
#include <QResizeEvent>

#include <QDebug>

#include "chartwidget.h"

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
}

void ChartWidget::paintEvent(QPaintEvent *) {
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
        int i = 0;
        for (size_t n=0; n<canvas_.numCharts(); ++n) {
            painter.drawText(5, i+15, QString::number(n));
            i += canvas_.chart(n).h();
            if (n != canvas_.numCharts()-1)
                painter.drawLine(QPoint{0, i}, QPoint{size_.width(), i});
        }
    }
}

void ChartWidget::resizeEvent(QResizeEvent *event) {
    size_ = event->size();
    canvas_.setCanvasSize({size_.width(), size_.height()});
}

