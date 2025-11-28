#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include "utils/config.h"
#include "utils/db_manager.h"
#include "utils/logger.h"
#include "ui/main_window/main_window.h" // 两个分支都有，只保留一个
#include "ui/db_qwidget/DbQWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

 // 初始化日志 (保留 feature/mty/db 的代码)
    /* Logger::getInstance().init("lanchat.log");
    Logger::getInstance().warning("111");*/

    //// 初始化配置 (保留 feature/mty/db 的代码)
    //Config::getInstance().load("config.ini");

    //// 尝试读取数据库路径...
    //QString dbPath = Config::getInstance().getString("database/path", QString());

    // 初始化数据库 (保留 feature/mty/db 的代码)
    if (!DatabaseManager::getInstance().init(QStringLiteral("C:\\mty\\LanChat\\src\\model\\lanchat.db"))) {
        //Logger::getInstance().error("");
        qDebug().noquote() << "Failed to initialize database";
    }
    else {
        //Logger::getInstance().log("Database initialized");
        qDebug().noquote() << "Database initialized";
    }
    
    /* 临时 UI 代码（不保留，或注释掉）*/
    
    // 显示 DbQWidget 进行调试 (根据需要决定是否保留)
    //DbQWidget dbQWidget;
    //dbQWidget.show();
    
    // 显示主窗口 (来自 dev 分支，这是应用最终的入口)
    MainWindow::instance()->show();
    
    // 关闭当前线程的 DB 连接 (保留 feature/mty/db 的代码)
    DatabaseManager::getInstance().closeConnectionForCurrentThread();
    //Logger::getInstance().close();
    return app.exec();
}
