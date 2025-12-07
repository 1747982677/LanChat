#ifndef NETWORKERRORDIALOG_H
#define NETWORKERRORDIALOG_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class NetworkErrorDialog;
}
QT_END_NAMESPACE

class NetworkErrorDialog : public QMainWindow
{
    Q_OBJECT

public:
    NetworkErrorDialog(QWidget *parent = nullptr);
    ~NetworkErrorDialog();

private:
    Ui::NetworkErrorDialog *ui;
};
#endif // NETWORKERRORDIALOG_H
