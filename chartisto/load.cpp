#include "config.h"
#include "load.h"
#include "ui_load.h"
#include "windowlist.h"

Load::Load(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Load)
{
    ui->setupUi(this);
    readWindowSets();

    Config config;
    const auto g = config.getLoadWindowGeometry();
    setGeometry(g.x, g.y, g.w, g.h);
}

Load::~Load() {
    delete ui;
}

void Load::readWindowSets() {
    ui->listWidget->clear();
    Config config;
    config.iterateWindowSets([&] (const std::string &windowSet) {
        ui->listWidget->addItem(QString::fromStdString(windowSet));
    });
}

void Load::closeEvent(QCloseEvent *) {
    const auto g = geometry();
    Config config;
    config.setLoadWindowGeometry({
        g.topLeft().x(),
        g.topLeft().y(),
        g.width(),
        g.height()
     });
}

void Load::on_pushButton_clicked() {
    WindowList::instance().load(ui->listWidget->currentItem()->text());
    activateWindow();
}
