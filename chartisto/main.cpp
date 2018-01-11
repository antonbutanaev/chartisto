#include <functional>

#include "mainwindow.h"
#include "windowlist.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    WindowList::instance().open();
    const auto rc = a.exec();
    WindowList::instance().clear();
    return rc;
}
