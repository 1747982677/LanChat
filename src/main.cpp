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
#include "ui/login/login_window.h"
#include "service/auth_service.h"
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
    
    
    // 检查是否已登录（有有效的 Token）
    AuthService& authService = AuthService::getInstance();
    if (authService.isLoggedIn()) {
        // 如果已登录，直接显示主窗口
        MainWindow::instance()->show();
    } else {
        // 如果未登录，显示登录窗口
        LoginWindow* loginWindow = new LoginWindow();
        
        // 连接登录成功信号，登录成功后显示主窗口
        QObject::connect(loginWindow, &LoginWindow::loginSucceeded, 
                        [loginWindow]() {
                            qDebug() << "main.cpp: 收到 loginSucceeded 信号";
                            // 登录成功后，隐藏登录窗口，显示主窗口
                            loginWindow->hide();
                            qDebug() << "main.cpp: 隐藏登录窗口，准备显示主窗口";
                            MainWindow::instance()->show();
                            qDebug() << "main.cpp: 主窗口已显示";
                        });
        
        loginWindow->show();
    }
    
    //// 关闭当前线程的 DB 连接 (保留 feature/mty/db 的代码)
    //DatabaseManager::getInstance().closeConnectionForCurrentThread();
}
