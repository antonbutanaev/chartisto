#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <list>
#include <memory>

#include <QTimer>

class MainWindow;
class Load;

class WindowList : public QObject {
    Q_OBJECT
private:
    WindowList();
    ~WindowList();
public:
    static WindowList &instance();
    void add(std::unique_ptr<MainWindow> &&);
    void open();
    void quit();
    void clear();
    void saveAs(const QString &);
    void load(const QString &);
    void showLoad();
private slots:
    void onTimeout();
private:
    enum class SaveMethod {JustSave, SaveAndClose};
    void save(SaveMethod);

    std::list<std::unique_ptr<MainWindow>> windowList_;
    std::unique_ptr<Load> loadWindow_;
    QTimer timer_;
};

#endif // WINDOWLIST_H
