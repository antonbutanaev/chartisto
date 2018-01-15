#ifndef SAVEAS_H
#define SAVEAS_H

#include <QDialog>
#include "geometryremember.h"

namespace Ui {
class SaveAs;
}

class SaveAs : public GeometryRemember<QDialog>
{
    Q_OBJECT

public:
    explicit SaveAs(QWidget *parent = nullptr);
    ~SaveAs();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SaveAs *ui;
};

#endif // SAVEAS_H
