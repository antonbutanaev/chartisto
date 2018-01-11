#include "macdform.h"
#include "windowlist.h"
#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include <QDebug>

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
    WindowList::instance().add(std::move(macdForm));
}

void MainWindow::on_actionLoad_triggered()
{
    static int n = 0;
    ++n;
    qDebug() << "Open " << n << " new windows";
    for (int i = 0; i < n; ++i) {
        auto window = std::make_unique<MainWindow>();
        window->setWindowTitle("Window " + QString::number(n) + "/" + QString::number(i));
        window->show();
        WindowList::instance().add(std::move(window));
    }
    qDebug() << "Close current window";
    close();
}

void MainWindow::on_actionQuit_triggered() {
    WindowList::instance().quit();
    qDebug() << "quit";
    //close();
}

void MainWindow::on_actionNew_triggered() {
    auto window = std::make_unique<MainWindow>();
    window->show();
    WindowList::instance().add(std::move(window));
}
