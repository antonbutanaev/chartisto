#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include "windowlist.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myAppTranslator;
    myAppTranslator.load(":/chartisto_" + QLocale::system().name());
    app.installTranslator(&myAppTranslator);

    WindowList::instance().open();
    const auto rc = app.exec();
    WindowList::instance().clear();
    return rc;
}
