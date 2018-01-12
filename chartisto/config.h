#ifndef CONFIG_H
#define CONFIG_H

#include <functional>
#include <string>
#include <json/json.h>

class Config
{
public:
    Config();
    ~Config();

    void save();
    bool hasCurrentWindowSet();

    using GetGeometry = std::function<void(int x, int y, int w, int h)>;
    void iterateCurrentWindowSet(GetGeometry);
    void clearCurrentWindowSet();
    void addToCurrentWindowSet(int x, int y, int w, int h);
    void setCurrentWindowSet(const std::string&);
    void iterateWindowSets(const std::function<void(const std::string &)> &);
private:
    Json::Value config_;
};

#endif // CONFIG_H
