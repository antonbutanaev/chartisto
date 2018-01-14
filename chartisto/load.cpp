#include "config.h"
#include "load.h"
#include "ui_load.h"
#include "windowlist.h"

Load::Load() :
    GeometryRemember<QDialog>("loadWindow"),
    ui(new Ui::Load)
{
    ui->setupUi(this);
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtons()));
    readWindowSets();
}

Load::~Load() {
    delete ui;
}

void Load::readWindowSets() {
    ui->listWidget->clear();
    Config config;
    const auto currentWindowSet = config.getCurrentWindowSet();
    setWindowTitle(tr("Replace") + " " + currentWindowSet.c_str());
    config.iterateWindowSets([&] (const std::string &windowSet) {
        if (windowSet != currentWindowSet)
            ui->listWidget->addItem(QString::fromStdString(windowSet));
    });
}

void Load::on_loadButton_clicked() {
    processSelected([&] (QString text) {
        WindowList::instance().load(text);
        readWindowSets();
        activateWindow();
    });
}

void Load::on_removeButton_clicked() {
    processSelected([&] (QString text) {
        Config().removeWindowSet(text.toStdString());
        readWindowSets();
    });
}

void Load::updateButtons() {
    const auto noSelectedItem = ui->listWidget->selectedItems().empty();
    ui->loadButton->setEnabled(!noSelectedItem);
    ui->removeButton->setEnabled(!noSelectedItem);

}

void Load::processSelected(std::function<void (QString)> cb) {
    const auto selected = ui->listWidget->selectedItems();
    if (!selected.empty())
        cb(selected.front()->text());
}
