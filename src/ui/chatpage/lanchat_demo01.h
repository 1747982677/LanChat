#ifndef LANCHAT_DEMO_1_H
#define LANCHAT_DEMO_1_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class LanChat_demo01;
}
QT_END_NAMESPACE

class LanChat_demo01 : public QMainWindow
{
    Q_OBJECT

public:
    LanChat_demo01(QWidget *parent = nullptr);
    ~LanChat_demo01();

private:
    Ui::LanChat_demo01 *ui;
};
#endif // LANCHAT_DEMO_1_H
