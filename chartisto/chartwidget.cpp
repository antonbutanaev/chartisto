#include <QPainter>
#include <QResizeEvent>

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

    QRect rect(0, 0, size_.width(), size_.height());
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    //painter.drawLine(rect.bottomLeft(), rect.topRight());
}

void ChartWidget::resizeEvent(QResizeEvent *event) {
    size_ = event->size();
    painter_.setCanvasSize({size_.width(), size_.height()});
}

