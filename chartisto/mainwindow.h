#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionMACD_triggered();

    void on_actionLoad_triggered();

    void on_actionQuit_triggered();

    void on_actionNew_triggered();

    void on_actionSave_as_triggered();

    void on_actionAdd_instrument_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
