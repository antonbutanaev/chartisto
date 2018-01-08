#include "macdform.h"
#include "ui_macdform.h"

MACDForm::MACDForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MACDForm)
{
    ui->setupUi(this);
}

MACDForm::~MACDForm()
{
    delete ui;
}
