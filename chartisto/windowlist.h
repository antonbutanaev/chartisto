#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <list>
#include <memory>
#include <QWidget>

class WindowList {
private:
    WindowList();
public:
    static WindowList &instance();
    void add(std::unique_ptr<QWidget>);
private:
    std::list<std::unique_ptr<QWidget>> windowList_;
};

#endif // WINDOWLIST_H
