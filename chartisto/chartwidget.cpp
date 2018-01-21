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
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    //painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.drawLine(0, 10, size_.width(), 10);
    QRect rect2(20, 20, 40, 40);
    painter.drawRect(rect2);

    int i = 0;
    qDebug() << "GGGG " << canvas_.numCharts();
    for (size_t n=0; n<canvas_.numCharts(); ++n) {

        i += canvas_.chart(n).h();
        qDebug() << "GGGG i=" << i << " " << canvas_.chart(n).h();
        painter.drawLine(QPoint{0, i}, QPoint{size_.width(), i});

    }

}

void ChartWidget::resizeEvent(QResizeEvent *event) {
    size_ = event->size();
    canvas_.setCanvasSize({size_.width(), size_.height()});
}

