#include "chartwidget.h"

#include <QPainter>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{

}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect rect(10 , 10, 2000, 2000);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawLine(rect.topLeft(), rect.bottomRight());

}

