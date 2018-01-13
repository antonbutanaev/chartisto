#include "config.h"
#include "load.h"
#include "ui_load.h"
#include "windowlist.h"

Load::Load(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Load)
{
    ui->setupUi(this);

    Config config;
    config.iterateWindowSets([&] (const std::string &windowSet) {
        ui->listWidget->addItem(QString::fromStdString(windowSet));
    });
}

Load::~Load() {
    delete ui;
}

void Load::on_pushButton_clicked() {
    WindowList::instance().load(ui->listWidget->currentItem()->text());
    activateWindow();
}
