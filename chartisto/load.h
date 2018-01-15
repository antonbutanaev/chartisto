#ifndef LOAD_H
#define LOAD_H

#include <functional>
#include <QDialog>
#include "geometryremember.h"

namespace Ui {
class Load;
}

class Load : public GeometryRemember<QDialog>
{
    Q_OBJECT

public:
    explicit Load();
    ~Load();
    void readWindowSets();

private slots:
    void on_loadButton_clicked();
    void on_removeButton_clicked();
    void updateButtons();

private:
    void processSelected(std::function<void(QString)>);

    Ui::Load *ui;
};

#endif // LOAD_H
