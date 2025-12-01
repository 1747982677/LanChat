#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "utils/config.h"
#include "utils/db_manager.h"
#include "utils/logger.h"
#include "ui/main_window/main_window.h" // 两个分支都有，只保留一个
#include "ui/db_qwidget/DbQWidget.h"
#include "ui/login/login_window.h"
#include "service/auth_service.h"
#include "service/chat_service.h"
#include "core/app_context.h"


int main(int argc, char* argv[])
{
    // 初始化日志
    Logger::getInstance().init("lanchat.log");

    QApplication app(argc, argv);

    // 初始化数据库 (在创建 QApplication 后执行以保证 Qt 插件可用)
    if (!DatabaseManager::getInstance().init(QStringLiteral("C:\\mty\\LanChat\\src\\model\\lanchat.db"))) {
        qDebug().noquote() << "Failed to initialize database";
    } else {
        qDebug().noquote() << "Database initialized";
    }

    // 初始化应用上下文
    AppContext& context = AppContext::instance();

    // 设置数据库路径
    context.setDatabasePath("src\\model\\lanchat.db");

    // 初始化所有模块
    if (!context.initialize()) {
        qCritical() << "Failed to initialize application";
        return -1;
    }

    // 启动所有线程
    context.startAll();

    // 初始化数据库（在 Worker 线程中执行）
    context.dbLogicController()->initializeDatabase(context.databasePath());

    // 检查是否已登录（有有效的 Token）
    AuthService& authService = AuthService::getInstance();
    if (authService.isLoggedIn()) {
        // 如果已登录，直接显示主窗口
        MainWindow::instance()->show();

        // Debug: simulate incoming message to test unread count
        #ifdef QT_DEBUG
        QTimer::singleShot(1500, [](){
            LanChat::Message msg;
            msg.senderId = QString("user_张三");
            msg.receiverId = QString("local_user");
            msg.content = QStringLiteral("测试未读消息");
            msg.timestamp = QDateTime::currentMSecsSinceEpoch();
            ChatService::getInstance().receiveMessage(msg);
        });
        #endif

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

                            // Debug: simulate incoming message after login to test unread count
                            #ifdef QT_DEBUG
                            QTimer::singleShot(1500, [](){
                                LanChat::Message msg;
                                msg.senderId = QString("user_张三");
                                msg.receiverId = QString("local_user");
                                msg.content = QStringLiteral("测试未读消息");
                                msg.timestamp = QDateTime::currentMSecsSinceEpoch();
                                ChatService::getInstance().receiveMessage(msg);
                            });
                            #endif
                        });

        loginWindow->show();
    }

    int result = app.exec();

    // 停止所有线程
    context.stopAll();

    return result;
}
