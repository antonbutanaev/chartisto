#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <chart/chart.h>

class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);

    void addChart() {canvas_.addChart(chart::Chart());}

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:

public slots:
private:
    QSize size_;
    chart::Canvas canvas_;
};

#endif // CHARTWIDGET_H
