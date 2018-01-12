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
    void open();
    void quit();
    void clear() {windowList_.clear();}
    void saveAs(const QString &);
    void load(const QString &);
private slots:
    void onTimeout();
private:
    enum class SaveMethod {JustSave, SaveAndClose};
    void save(SaveMethod);

    std::list<std::unique_ptr<QWidget>> windowList_;
    QTimer timer_;
};

#endif // WINDOWLIST_H
