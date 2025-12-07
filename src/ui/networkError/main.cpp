#include "networkerrordialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NetworkErrorDialog w;
    w.show();
    return a.exec();
}
