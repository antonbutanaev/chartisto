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
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:
private:
    QSize size_;
    chart::Canvas canvas_;
    bool leftPressed_ = false;
    int leftPressedAt_ = -1;
    int chartNumOver_ = -1;
    int chartResized_ = -1;
};

#endif // CHARTWIDGET_H
