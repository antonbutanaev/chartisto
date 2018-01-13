#include "saveas.h"
#include "ui_saveas.h"
#include "windowlist.h"

SaveAs::SaveAs(QWidget *parent) :
    GeometryRemember<QDialog>("saveAsWindow", parent),
    ui(new Ui::SaveAs)
{
    ui->setupUi(this);
    const auto currentWindowSet = QString::fromStdString(Config().getCurrentWindowSet());
    ui->saveAs->setText(currentWindowSet);
    ui->saveAs->setSelection(0, currentWindowSet.length());
}

SaveAs::~SaveAs() {
    delete ui;
}

void SaveAs::on_pushButton_clicked() {
    WindowList::instance().saveAs(ui->saveAs->text());
    close();
}
