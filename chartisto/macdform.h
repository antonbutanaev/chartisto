#ifndef MACDFORM_H
#define MACDFORM_H

#include <QDialog>

namespace Ui {
class MACDForm;
}

class MACDForm : public QDialog
{
    Q_OBJECT

public:
    explicit MACDForm(QWidget *parent = 0);
    ~MACDForm();

private:
    Ui::MACDForm *ui;
};

#endif // MACDFORM_H
