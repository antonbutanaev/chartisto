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
        cb(
            it[tag::x].asInt(),
            it[tag::y].asInt(),
            it[tag::width].asInt(),
            it[tag::height].asInt()
        );
}

void Config::clearCurrentWindowSet() {
    if (!config_.isMember(tag::currentWindowSet))
        config_[tag::currentWindowSet] = tag::noName;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].clear();
}

void Config::addToCurrentWindowSet(int x, int y, int w, int h) {
    Json::Value window;
    window[tag::x] = x;
    window[tag::y] = y;
    window[tag::width] = w;
    window[tag::height] = h;
    config_[tag::windowSets][config_[tag::currentWindowSet].asString()].append(window);
}

void Config::setCurrentWindowSet(const std::string &windowSet) {
    config_[tag::currentWindowSet] = windowSet;
}
