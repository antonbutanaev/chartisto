#include "mainwindow.h"
#include "windowlist.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <QApplication>
#include <QDebug>


namespace {

const char *configFile() {
    static std::string file = std::getenv("HOME") + std::string("/.chartisto");
    return file.c_str();
}

}

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
    const auto openBareMainWindow = [&] {
        auto mainWindow = std::make_unique<MainWindow>();
        mainWindow->show();
        add(std::move(mainWindow));
    };

    std::ifstream ifs(configFile());
    if (!ifs)
        openBareMainWindow();
    else {
        Json::Value root;
        ifs >> root;
        std::stringstream ss;
        ss << root;
        qDebug() << "GGGG " << ss.str().c_str();
        if (root["currentWindowSet"].isNull()) {
            qDebug() << "GGGG2";
            openBareMainWindow();
        }
        else {
            qDebug() << "GGGG3";
            for (const auto &it: root["windowSets"][root["currentWindowSet"].asString()]) {
                auto mainWindow = std::make_unique<MainWindow>();
                qDebug() << "GGGG4" << it["x"].asInt() << it["y"].asInt() << it["width"].asInt() << it["height"].asInt();
                mainWindow->setGeometry(it["x"].asInt(), it["y"].asInt(), it["width"].asInt(), it["height"].asInt());
                mainWindow->show();
                add(std::move(mainWindow));
            }
        }
    }
}

void WindowList::save() {
}

void WindowList::quit() {
    Json::Value root;
    std::ifstream ifs(configFile());
    ifs >> root;

    auto &windowList = root["windowSets"][root["currentWindowSet"].asString()];
    windowList.clear();

    for (const auto &it: windowList_) {

        Json::Value window;
        window["x"] = it->geometry().topLeft().x();
        window["y"] = it->geometry().topLeft().y();
        window["width"] = it->geometry().width();
        window["height"] = it->geometry().height();
        windowList.append(window);

        //it->close();

        std::stringstream ss;
        ss << windowList;
        qDebug() << "GGGGw " << ss.str().c_str();

    }

    //windowList_.clear();

    std::ofstream ofs(configFile());
    ofs << root;

    //QApplication::quit();

}

void WindowList::onTimeout() {
    //qDebug() << "onTimeout " << windowList_.size();

    for (auto it = windowList_.begin(); it != windowList_.end(); ) {
        if (!(*it)->isVisible())
            windowList_.erase(it++);
        else
            ++it;
    }
}
