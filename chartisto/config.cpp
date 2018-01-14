#include <iostream>
#include <fstream>
#include <stdexcept>
#include "config.h"

namespace {

const char *configFile() {
    static std::string file = std::getenv("HOME") + std::string("/.chartisto.json");
    return file.c_str();
}

namespace tag {
constexpr char currentWindowSet[] = "currentWindowSet";
constexpr char windowSets[] = "windowSets";
constexpr char x[] = "x";
constexpr char y[] = "y";
constexpr char w[] = "w";
constexpr char h[] = "h";
constexpr char geometry[] = "geometry";
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

void Config::iterateCurrentWindowSet(GetGeometry cb) const {
    for (const auto &it: config_[tag::windowSets][config_[tag::currentWindowSet].asString()]) {
        const auto &geometry = it[tag::geometry];
        cb({
            geometry[tag::x].asInt(),
            geometry[tag::y].asInt(),
            geometry[tag::w].asInt(),
            geometry[tag::h].asInt()
        });
    }
}

void Config::clearCurrentWindowSet(std::string noName) {
    if (!config_.isMember(tag::currentWindowSet))
        config_[tag::currentWindowSet] = noName;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].clear();
}

void Config::addToCurrentWindowSet(Geometry g) {
    Json::Value window;
    auto &geometry = window[tag::geometry];
    geometry[tag::x] = g.x;
    geometry[tag::y] = g.y;
    geometry[tag::w] = g.w;
    geometry[tag::h] = g.h;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].append(window);
}

void Config::setCurrentWindowSet(std::string windowSet) {
    config_[tag::currentWindowSet] = windowSet;
}

void Config::iterateWindowSets(std::function<void(std::string)> cb) const {
    const auto &windowSets = config_[tag::windowSets];
    for (auto it = windowSets.begin(); it != windowSets.end(); ++it)
        cb(it.key().asString());
}

Config::Geometry Config::getWindowGeometry(std::string windowType) const {
    Geometry g = {};
    if (config_.isMember(windowType)) {
        const auto &geometry = config_[windowType][tag::geometry];
        g.x = geometry[tag::x].asInt();
        g.y = geometry[tag::y].asInt();
        g.w = geometry[tag::w].asInt();
        g.h = geometry[tag::h].asInt();
    }
    return g;
}

void Config::setWindowGeometry(std::string windowType, Geometry g) {
    auto &geometry = config_[windowType][tag::geometry];
    geometry[tag::x] = g.x;
    geometry[tag::y] = g.y;
    geometry[tag::w] = g.w;
    geometry[tag::h] = g.h;
}

std::string Config::getCurrentWindowSet() const {
    return config_[tag::currentWindowSet].asString();
}

void Config::removeWindowSet(std::string window) {
    Json::Value removed;
    config_[tag::windowSets].removeMember(window, &removed);
}
