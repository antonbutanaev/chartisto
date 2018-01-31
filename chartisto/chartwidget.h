#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <chart/chart.h>

class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    using Pos = size_t;
    using Coord = int;

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
    bool leftButtonPressed_ = false;
    static const auto NoCoord = std::numeric_limits<Coord>::lowest();
    Coord leftButtonPressedAt_ = NoCoord;
    Coord draggingHere_ = NoCoord;

    static const auto NoPos = std::numeric_limits<Pos>::max();
    Pos chartNumOver_ = NoPos;
    Pos chartResized_ = NoPos;
};

#endif
