#include "lanchat_demo01.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LanChat_demo01 w;
    w.show();
    return a.exec();
}
