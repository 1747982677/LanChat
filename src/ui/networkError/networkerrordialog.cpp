#include "networkerrordialog.h"
#include "./ui_networkerrordialog.h"

NetworkErrorDialog::NetworkErrorDialog(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NetworkErrorDialog)
{
    ui->setupUi(this);
}

NetworkErrorDialog::~NetworkErrorDialog()
{
    delete ui;
}
