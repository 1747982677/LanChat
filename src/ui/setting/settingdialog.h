#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingDialog; }
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget* parent = nullptr);
    ~SettingDialog();
   
private slots:
    void on_btnChangeStoragePath_clicked();  // change path button
    void on_btnClearStorage_clicked();       // clear button
    void on_pushButton_3_clicked();          // logout button
    
private:
    Ui::SettingDialog* ui;
};

#endif // SETTINGDIALOG_H
