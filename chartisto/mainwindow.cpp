#include "macdform.h"
#include "windowlist.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    std::unique_ptr<QWidget> macdForm(new MACDForm(this));
    macdForm->show();
    WindowList::instance().add(std::move(macdForm));
}
