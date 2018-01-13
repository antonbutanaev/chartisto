#include <cstdlib>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <QApplication>
#include <QDebug>

#include "config.h"
#include "mainwindow.h"
#include "load.h"
#include "windowlist.h"

WindowList::WindowList() : timer_(this) {
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer_.start(500);
}

WindowList::~WindowList() {
}

WindowList &WindowList::instance() {
    static WindowList windowList;
    return windowList;
}

void WindowList::add(std::unique_ptr<MainWindow> &&widget) {
    windowList_.push_back(std::move(widget));
}

void WindowList::open() {
    Config config;
    if (!config.hasCurrentWindowSet()) {
        auto mainWindow = std::make_unique<MainWindow>();
        mainWindow->show();
        add(std::move(mainWindow));
    } else {
        config.iterateCurrentWindowSet([&] (const Config::Geometry &g) {
            auto mainWindow = std::make_unique<MainWindow>();
            mainWindow->setGeometry(g.x, g.y, g.w, g.h);
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

        config.addToCurrentWindowSet({
            it->geometry().topLeft().x(),
            it->geometry().topLeft().y(),
            it->geometry().width(),
            it->geometry().height()
        });

        if (method == SaveMethod::SaveAndClose)
            it->close();
    }
}

void WindowList::quit() {
    save(SaveMethod::SaveAndClose);
    loadWindow_->close();
}

void WindowList::clear() {windowList_.clear();}

void WindowList::saveAs(const QString &windowSet) {
    Config().setCurrentWindowSet(windowSet.toStdString());
    save(SaveMethod::JustSave);
    if (loadWindow_)
        loadWindow_->readWindowSets();
}

void WindowList::load(const QString &x) {
    save(SaveMethod::SaveAndClose);
    Config config;
    config.setCurrentWindowSet(x.toStdString());
    config.iterateCurrentWindowSet([&] (const Config::Geometry &g) {
        auto mainWindow = std::make_unique<MainWindow>();
        mainWindow->setGeometry(g.x, g.y, g.w, g.h);
        mainWindow->show();
        add(std::move(mainWindow));
    });
}

void WindowList::showLoad() {
    if (!loadWindow_)
        loadWindow_.reset(new Load);


    loadWindow_->readWindowSets();
    loadWindow_->show();
    loadWindow_->activateWindow();
}

void WindowList::onTimeout() {
    for (auto it = windowList_.begin(); it != windowList_.end(); ) {
        if (!(*it)->isVisible())
            windowList_.erase(it++);
        else
            ++it;
    }
}
