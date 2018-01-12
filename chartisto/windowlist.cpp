#include <cstdlib>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "windowlist.h"

namespace {

const char *configFile() {
    static std::string file = std::getenv("HOME") + std::string("/.chartisto");
    return file.c_str();
}

namespace tag {
constexpr char currentWindowSet[] = "currentWindowSet";
constexpr char windowSets[] = "windowSets";
constexpr char x[] = "x";
constexpr char y[] = "y";
constexpr char width[] = "width";
constexpr char height[] = "height";
}

auto getConfig() {
    Json::Value root;
    std::ifstream ifs(configFile());
    if (!!ifs) {
        ifs.seekg(0, std::ios::end);
        if (0 != ifs.tellg()) {
            ifs.seekg(0);
            ifs >> root;
        }
    }
    return root;
}

void saveConfig(const Json::Value &root) {
    std::ofstream ofs(configFile());
    ofs << root << std::endl;
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

    const auto root = getConfig();
    if (!root.isMember(tag::currentWindowSet))
        openBareMainWindow();
    else
        for (const auto &it: root[tag::windowSets][root[tag::currentWindowSet].asString()]) {
            auto mainWindow = std::make_unique<MainWindow>();
            mainWindow->setGeometry(
                it[tag::x].asInt(),
                it[tag::y].asInt(),
                it[tag::width].asInt(),
                it[tag::height].asInt()
            );
            mainWindow->show();
            add(std::move(mainWindow));
        }
}

void WindowList::save(SaveMethod method) {
    auto root = getConfig();
    if (!root.isMember(tag::currentWindowSet))
        root[tag::currentWindowSet] = "default";

    auto &windowList = root[tag::windowSets][root[tag::currentWindowSet].asString()];
    windowList.clear();

    for (const auto &it: windowList_) {
        if (!dynamic_cast<MainWindow*>(&*it)) // FIXME
            continue;
        Json::Value window;
        window[tag::x] = it->geometry().topLeft().x();
        window[tag::y] = it->geometry().topLeft().y();
        window[tag::width] = it->geometry().width();
        window[tag::height] = it->geometry().height();
        windowList.append(window);

        if (method == SaveMethod::SaveAndClose)
            it->close();
    }

    saveConfig(root);
}

void WindowList::quit() {
    save(SaveMethod::SaveAndClose);
}

void WindowList::saveAs(const QString &windowSet)
{
    auto config = getConfig();
    config[tag::currentWindowSet] = windowSet.toStdString();
    saveConfig(config);
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
