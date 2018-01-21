#include <memory>

#include "macdform.h"
#include "windowlist.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "saveas.h"
#include "load.h"
#include "config.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionMACD_triggered() {
    auto macdForm = std::make_unique<MACDForm>(this);
    macdForm->show();
    //WindowList::instance().add(std::move(macdForm));
}

void MainWindow::on_actionLoad_triggered() {
    WindowList::instance().showLoad();
}

void MainWindow::on_actionQuit_triggered() {
    WindowList::instance().quit();
}

void MainWindow::on_actionNew_triggered() {
    auto window = std::make_unique<MainWindow>();
    window->show();
    WindowList::instance().add(std::move(window));
}

void MainWindow::on_actionSave_as_triggered() {
    SaveAs().exec();
}
