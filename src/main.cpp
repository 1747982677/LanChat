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
#include "core/app_context.h"

int main(int argc, char* argv[])
{

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
    

    QApplication app(argc, argv);

    // 初始化应用上下文
    AppContext& context = AppContext::instance();

    // 设置数据库路径
    context.setDatabasePath("src\model\lanchat.db");

    // 初始化所有模块
    if (!context.initialize()) {
        qCritical() << "Failed to initialize application";
        return -1;
    }

    // 启动所有线程
    context.startAll();

    // 初始化数据库（在 Worker 线程中执行）
    context.dbLogicController()->initializeDatabase(context.databasePath());

    // 显示主窗口
    MainWindow::instance()->show();

    int result = app.exec();

    // 停止所有线程
    context.stopAll();

    return result;
    
    
    
    //// 关闭当前线程的 DB 连接 (保留 feature/mty/db 的代码)
    //DatabaseManager::getInstance().closeConnectionForCurrentThread();
}
