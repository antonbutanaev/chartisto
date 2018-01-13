#ifndef LOAD_H
#define LOAD_H

#include "geometryremember.h"
#include <QDialog>

namespace Ui {
class Load;
}

class QCloseEvent;

class Load : public GeometryRemember<QDialog>
{
    Q_OBJECT

public:
    explicit Load();
    ~Load();

    void readWindowSets();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Load *ui;
};

#endif // LOAD_H
