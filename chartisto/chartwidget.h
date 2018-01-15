#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <painter.h>

class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

signals:

public slots:
private:
    QSize size_;
    Painter painter_;
};

#endif // CHARTWIDGET_H
