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

    using GetGeometry = std::function<void(const Geometry&)>;
    void iterateCurrentWindowSet(GetGeometry) const;
    void clearCurrentWindowSet();
    void addToCurrentWindowSet(const Geometry&);
    void setCurrentWindowSet(const std::string&);
    void iterateWindowSets(const std::function<void(const std::string &)> &) const;

    Geometry getWindowGeometry(const std::string &type) const;
    void setWindowGeometry(const std::string &type, const Geometry &);
    std::string getCurrentWindowSet() const;

    void removeWindowSet(const std::string &type);
private:
    Json::Value config_;
};

#endif // CONFIG_H
