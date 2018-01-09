#include "windowlist.h"
#include <QDebug>

WindowList::WindowList() : timer_(this) {
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer_.start(500);
}

WindowList &WindowList::instance() {
    static WindowList windowList;
    return windowList;
}

void WindowList::add(std::unique_ptr<QWidget> widget) {
    windowList_.push_back(std::move(widget));
}

void WindowList::onTimeout() {
    qDebug() << "onTimeout " << windowList_.size();

    for (auto it = windowList_.begin(); it != windowList_.end(); ) {
        if (!(*it)->isVisible())
            windowList_.erase(it++);
        else
            ++it;
    }

}
