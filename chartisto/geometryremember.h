#ifndef GEOMETRYREMEMBER_H
#define GEOMETRYREMEMBER_H

#include <string>
#include "config.h"

class QShowEvent;
class QCloseEvent;

template <class T> class GeometryRemember : public T {
public:
    GeometryRemember(std::string &&type) : type_(std::move(type)) { }
private:
    void showEvent(QShowEvent *ev) override {
        T::showEvent(ev);
        const auto g = Config().getWindowGeometry(type_);
        T::setGeometry(g.x, g.y, g.w, g.h);
    }

    void closeEvent(QCloseEvent *ev) override {
        T::closeEvent(ev);
        const auto g = T::geometry();
        Config config;
        config.setWindowGeometry(type_, {
            g.topLeft().x(),
            g.topLeft().y(),
            g.width(),
            g.height()
         });
    }
private:
    std::string type_;
};

#endif // GEOMETRYREMEMBER_H
