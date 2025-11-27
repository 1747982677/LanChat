#include <QApplication>
#include "ui/main_window/main_window.h"


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow::instance()->show();


    return app.exec();
}
