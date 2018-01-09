#include "macdform.h"
#include "windowlist.h"
#include "mainwindow.h"
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
    std::unique_ptr<QWidget> macdForm(new MACDForm(this));
    macdForm->show();
    WindowList::instance().add(std::move(macdForm));
}

void MainWindow::on_actionLoad_triggered()
{
    static int n = 0;
    ++n;
    qDebug() << "Open " << n << " new windows";
    for (int i = 0; i < n; ++i) {
        std::unique_ptr<MainWindow> mw(new MainWindow);
        mw->setWindowTitle("Window " + QString::number(n) + "/" + QString::number(i));
        mw->show();
        WindowList::instance().add(std::move(mw));
    }
    qDebug() << "Close current window";
    close();
}
