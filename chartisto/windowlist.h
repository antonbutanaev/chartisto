#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <list>
#include <memory>
#include <QWidget>
#include <QTimer>

class WindowList : public QObject {
    Q_OBJECT
private:
    WindowList();
public:
    static WindowList &instance();
    void add(std::unique_ptr<QWidget> &&);
private slots:
    void onTimeout();
private:
    std::list<std::unique_ptr<QWidget>> windowList_;
    QTimer timer_;
};

#endif // WINDOWLIST_H
