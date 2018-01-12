#include "load.h"
#include "ui_load.h"
#include "windowlist.h"

Load::Load(QStringList &&items, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Load),
    items_(std::move(items))
{
    ui->setupUi(this);
    ui->listWidget->addItems(items_);
}

Load::~Load() {
    delete ui;
}

void Load::on_pushButton_clicked() {
    WindowList::instance().load(ui->listWidget->currentItem()->text());
    close();
}
