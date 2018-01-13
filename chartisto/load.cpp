#include "config.h"
#include "load.h"
#include "ui_load.h"
#include "windowlist.h"

Load::Load() :
    GeometryRemember<QDialog>("loadWindow"),
    ui(new Ui::Load)
{
    ui->setupUi(this);
    readWindowSets();
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

void Load::on_pushButton_clicked() {
    WindowList::instance().load(ui->listWidget->currentItem()->text());
    activateWindow();
}
