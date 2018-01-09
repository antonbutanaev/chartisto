#include "windowlist.h"

WindowList::WindowList() {

}

WindowList &WindowList::instance() {
    static WindowList windowList;
    return windowList;
}

void WindowList::add(std::unique_ptr<QWidget> widget) {
    windowList_.push_back(std::move(widget));
}
