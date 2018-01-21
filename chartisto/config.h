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

    struct Geometry {
        int x, y, w, h;
    };

    using GetGeometry = std::function<void(Geometry)>;
    void iterateCurrentWindowSet(GetGeometry) const;
    void clearCurrentWindowSet(std::string noName);
    void addToCurrentWindowSet(Geometry);
    void setCurrentWindowSet(std::string);
    void iterateWindowSets(std::function<void(std::string)>) const;

    Geometry getWindowGeometry(std::string windowType) const;
    void setWindowGeometry(std::string window, Geometry);
    std::string getCurrentWindowSet() const;

    void removeWindowSet(std::string window);
private:
    Json::Value config_;
    static int instanceCounter_;
};

#endif // CONFIG_H
