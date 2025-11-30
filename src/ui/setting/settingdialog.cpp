#include "settingdialog.h"
#include "ui_settingdialog.h"

#include <QDebug>

SettingDialog::SettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::on_btnChangeStoragePath_clicked()
{
    qDebug() << "[SettingDialog] ChangeStoragePath clicked";
}

void SettingDialog::on_btnClearStorage_clicked()
{
    qDebug() << "[SettingDialog] ClearStorage clicked";
}

void SettingDialog::on_pushButton_3_clicked()
{
    qDebug() << "[SettingDialog] Logout clicked";
    accept();   // close dialog
}
