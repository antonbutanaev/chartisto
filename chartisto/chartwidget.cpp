#include "chartwidget.h"

#include <QPainter>
#include <QDebug>
#include <QResizeEvent>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{

}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPalette pal = palette();

    // set black background
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    QPainter painter(this);
    QRect rect(0, 0, size_.width(), size_.height());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRect(rect);
}

void ChartWidget::resizeEvent(QResizeEvent *event) {
    qDebug() << "Resize " << event->size();
    size_ = event->size();
}

