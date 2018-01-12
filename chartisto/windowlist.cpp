#include <cstdlib>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <QApplication>
#include <QDebug>

#include "config.h"
#include "mainwindow.h"
#include "windowlist.h"

WindowList::WindowList() : timer_(this) {
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer_.start(500);
}

WindowList &WindowList::instance() {
    static WindowList windowList;
    return windowList;
}

void WindowList::add(std::unique_ptr<QWidget> &&widget) {
    windowList_.push_back(std::move(widget));
}

void WindowList::open() {
    Config config;
    if (!config.hasCurrentWindowSet()) {
        auto mainWindow = std::make_unique<MainWindow>();
        mainWindow->show();
        add(std::move(mainWindow));
    } else {
        config.iterateCurrentWindowSet([&] (auto x, auto y, auto w, auto h) {
            auto mainWindow = std::make_unique<MainWindow>();
            mainWindow->setGeometry(x, y, w, h);
            mainWindow->show();
            add(std::move(mainWindow));
        });
    }
}

void WindowList::save(SaveMethod method) {
    Config config;
    config.clearCurrentWindowSet();
    for (const auto &it: windowList_) {
        if (!dynamic_cast<MainWindow*>(&*it)) // FIXME
            continue;

        config.addToCurrentWindowSet(
            it->geometry().topLeft().x(),
            it->geometry().topLeft().y(),
            it->geometry().width(),
            it->geometry().height()
        );

        if (method == SaveMethod::SaveAndClose)
            it->close();
    }
}

void WindowList::quit() {
    save(SaveMethod::SaveAndClose);
}

void WindowList::saveAs(const QString &windowSet) {
    Config().setCurrentWindowSet(windowSet.toStdString());
}

void WindowList::onTimeout() {
    for (auto it = windowList_.begin(); it != windowList_.end(); ) {
        if (!(*it)->isVisible())
            windowList_.erase(it++);
        else
            ++it;
    }
}
