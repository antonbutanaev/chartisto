#include <functional>

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>

#include "mainwindow.h"
#include "windowlist.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load(":/chartisto_" + QLocale::system().name());
    app.installTranslator(&myappTranslator);

    WindowList::instance().open();
    const auto rc = app.exec();
    WindowList::instance().clear();
    return rc;
}