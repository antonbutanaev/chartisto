#include <functional>

#include "mainwindow.h"
#include "windowlist.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    WindowList::instance().open();
    return a.exec();
}
