#ifndef GEOMETRYREMEMBER_H
#define GEOMETRYREMEMBER_H

#include <string>
#include "config.h"

class QWidget;
class QShowEvent;
class QCloseEvent;

template <class T> class GeometryRemember : public T {
public:
    GeometryRemember(std::string &&type, QWidget *parent = nullptr) : T(parent), type_(std::move(type)) {}
private:
    void showEvent(QShowEvent *event) override {
        T::showEvent(event);
        const auto g = Config().getWindowGeometry(type_);
        if (g.w && g.h)
            T::setGeometry(g.x, g.y, g.w, g.h);
    }

    void closeEvent(QCloseEvent *event) override {
        T::closeEvent(event);
        const auto g = T::geometry();
        Config().setWindowGeometry(type_, {
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
