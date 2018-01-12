#ifndef SAVEAS_H
#define SAVEAS_H

#include <QDialog>

namespace Ui {
class SaveAs;
}

class SaveAs : public QDialog
{
    Q_OBJECT

public:
    explicit SaveAs(QWidget *parent = 0);
    ~SaveAs();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SaveAs *ui;
};

#endif // SAVEAS_H
