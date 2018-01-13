#include <iostream>
#include <fstream>
#include <stdexcept>
#include "config.h"

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
constexpr char noName[] = "no name";
constexpr char loadWindow[] = "loadWindow";
}
}

Config::Config() {
    std::ifstream ifs(configFile());
    if (ifs)
        ifs >> config_;
}

Config::~Config() {
    try {
        save();
    } catch (const std::exception &x)  {
        std::cerr << "Save config error: " << x.what() << std::endl;
    }
}

void Config::save() {
    std::ofstream(configFile()) << config_ << std::endl;
}

bool Config::hasCurrentWindowSet()
{
    return config_.isMember(tag::currentWindowSet);
}

void Config::iterateCurrentWindowSet(GetGeometry cb) {
    for (const auto &it: config_[tag::windowSets][config_[tag::currentWindowSet].asString()])
        cb({
            it[tag::x].asInt(),
            it[tag::y].asInt(),
            it[tag::width].asInt(),
            it[tag::height].asInt()
        });
}

void Config::clearCurrentWindowSet() {
    if (!config_.isMember(tag::currentWindowSet))
        config_[tag::currentWindowSet] = tag::noName;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].clear();
}

void Config::addToCurrentWindowSet(const Geometry &g) {
    Json::Value window;
    window[tag::x] = g.x;
    window[tag::y] = g.y;
    window[tag::width] = g.w;
    window[tag::height] = g.h;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].append(window);
}

void Config::setCurrentWindowSet(const std::string &windowSet) {
    config_[tag::currentWindowSet] = windowSet;
}

void Config::iterateWindowSets(const std::function<void(const std::string &)> &cb) {
    const auto &windowSets = config_[tag::windowSets];
    for (auto it = windowSets.begin(); it != windowSets.end(); ++it)
        cb(it.key().asString());
}

Config::Geometry Config::getLoadWindowGeometry() {
    Geometry g = {};
    if (config_.isMember(tag::loadWindow)) {
        const auto &loadWindow = config_[tag::loadWindow];
        g.x = loadWindow[tag::x].asInt();
        g.y = loadWindow[tag::y].asInt();
        g.w = loadWindow[tag::width].asInt();
        g.h = loadWindow[tag::height].asInt();
    }
    return g;
}

void Config::setLoadWindowGeometry(const Geometry &g) {
    auto &loadWindow = config_[tag::loadWindow];
    loadWindow[tag::x] = g.x;
    loadWindow[tag::y] = g.y;
    loadWindow[tag::width] = g.w;
    loadWindow[tag::height] = g.h;
}
