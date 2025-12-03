#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QUuid>
#include "utils/config.h"
#include "utils/db_manager.h"
#include "utils/logger.h"
#include "ui/main_window/main_window.h"
#include "ui/db_qwidget/DbQWidget.h"
#include "ui/login/login_window.h"
#include "service/auth_service.h"
#include "service/chat_service.h"
#include "core/app_context.h"
#include "core/network_controller.h"
#include "common/types.h"


int main(int argc, char* argv[])
{
    // 初始化日志
    Logger::getInstance().init("lanchat.log");

    QApplication app(argc, argv);

    // 初始化数据库 (使用相对路径，避免绝对路径在不同机器上失效)
    QString dbPath = "lanchat.db"; // 默认放在运行目录下
    if (!DatabaseManager::getInstance().init(dbPath)) {
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

    // ==== 网络层测试代码 ====
    #ifdef QT_DEBUG
    // 等待 NetworkController Worker 初始化完成
    QObject::connect(context.networkController(), &NetworkController::initialized, [&context](){
        Logger::getInstance().log("[TEST] NetworkController Worker initialized! Starting test server...");
        
        // 1. 启动本地 WebSocket 服务器（端口 8086）
        Logger::getInstance().log("[TEST] Starting local WebSocket server on port 8086...");
        context.networkController()->startServer(8086);

        // 2. 延迟 1 秒后连接到本地服务器
        QTimer::singleShot(1000, [&context](){
            Logger::getInstance().log("[TEST] Connecting to local WebSocket server...");
            context.networkController()->connectToServer("127.0.0.1", 8086);
        });
    });
    #endif
    // ========================

    // 初始化数据库（在 Worker 线程中执行）
    context.dbLogicController()->initializeDatabase(context.databasePath());


    // 检查是否已登录（有有效的 Token）
    AuthService& authService = AuthService::getInstance();
    if (authService.isLoggedIn()) {
        // 如果已登录，直接显示主窗口
        MainWindow::instance()->show();

        // Debug: simulate incoming message to test unread count
        // #ifdef QT_DEBUG
        // QTimer::singleShot(1500, [](){
        //     LanChat::Message msg;
        //     msg.senderId = QString("user_张三");
        //     msg.receiverId = QString("local_user");
        //     msg.content = QStringLiteral("测试未读消息");
        //     msg.timestamp = QDateTime::currentMSecsSinceEpoch();
        //     ChatService::getInstance().receiveMessage(msg);
        // });
        // #endif

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
                           // #ifdef QT_DEBUG
                           // QTimer::singleShot(1500, [](){
                           //     LanChat::Message msg;
                           //     msg.senderId = QString("user_张三");
                           //     msg.receiverId = QString("local_user");
                           //     msg.content = QStringLiteral("测试未读消息");
                           //     msg.timestamp = QDateTime::currentMSecsSinceEpoch();
                           //     ChatService::getInstance().receiveMessage(msg);
                           // });

                           // // 网络层测试：发送一条消息到网络层，验证消息发送和接收
                           // QTimer::singleShot(3000, [](){
                           //     Logger::getInstance().log("[TEST] Sending test message through network layer...");
                           //     LanChat::Message msg;
                           //     msg.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
                           //     msg.senderId = "user_张三"; // 模拟张三发来的消息
                           //     msg.receiverId = "local_user";
                           //     msg.content = "【网络测试】这是通过 WebSocket 发送的消息！";
                           //     msg.type = LanChat::MessageType::Text;
                           //     msg.timestamp = QDateTime::currentMSecsSinceEpoch();
                                
                           //     NetworkController::instance().sendMessage(msg.toJson());
                           // });
                           // #endif
                        });

        loginWindow->show();
    }


    int result = app.exec();

    // 停止所有线程
    context.stopAll();

    return result;

    
    //// 关闭当前线程的 DB 连接 (保留 feature/mty/db 的代码)
    //DatabaseManager::getInstance().closeConnectionForCurrentThread();

}
