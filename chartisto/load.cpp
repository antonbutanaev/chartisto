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
    const auto currentWindowSet = config.getCurrentWindowSet();
    setWindowTitle(QString("Replace ") + currentWindowSet.c_str() + " with..");
    config.iterateWindowSets([&] (const std::string &windowSet) {
        if (windowSet != currentWindowSet)
            ui->listWidget->addItem(QString::fromStdString(windowSet));
    });
}

void Load::on_loadButton_clicked() {
    WindowList::instance().load(ui->listWidget->currentItem()->text());
    readWindowSets();
    activateWindow();

}

void Load::on_removeButton_clicked() {
    Config().removeWindowSet(ui->listWidget->currentItem()->text().toStdString());
    readWindowSets();

}
