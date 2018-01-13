#ifndef LOAD_H
#define LOAD_H

#include <QDialog>

namespace Ui {
class Load;
}

class QCloseEvent;

class Load : public QDialog
{
    Q_OBJECT

public:
    explicit Load(QWidget *parent = nullptr);
    ~Load() override;

    void readWindowSets();
    void closeEvent(QCloseEvent *) override;

private slots:
    void on_pushButton_clicked();

private:
    Ui::Load *ui;
};

#endif // LOAD_H
