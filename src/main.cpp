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
    //QString dbPath = "lanchat.db"; // 默认放在运行目录下
    //if (!DatabaseManager::getInstance().init(dbPath)) {
     //   qDebug().noquote() << "Failed to initialize database";
    //} else {
     //  qDebug().noquote() << "Database initialized";
    //}

    quintptr id = reinterpret_cast<quintptr>(QThread::currentThreadId());
    Logger::getInstance().log(QString("当前主线程：lanchat_conn_%1").arg(id));

    // 初始化应用上下文
    AppContext& context = AppContext::instance();
    // 设置数据库路径
    //需要根据实际路径修改
    context.setDatabasePath("C:\\Users\\caid0\\source\\repos\\LanChat\\src\\model\\lanchat.db");
    /*context.setDatabasePath("src\\model\\lanchat.db");*/

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
    QObject::connect(context.networkController(), &NetworkController::initialized, [&context]() {
        Logger::getInstance().log("[TEST] NetworkController Worker initialized! Starting test server...");

        // 1. 启动本地 chat服务，需要传入用户的唯一 ID
        QString usrID = "local_user"; // 本地测试用户 ID (一定要是唯一ID) 
        Logger::getInstance().log("[TEST] Starting local WebSocket server on port 8080...");
        //发送消息最好使用 LanChat::Message 结构体，调用networkController->sendMessage(LanChat::Message());
        //或者指定接收用户的ID调用 networkController->sendTextMessage(const QString& text, const QString& receiverId);
        });
#endif
    // ========================

    // 初始化数据库（在 Worker 线程中执行）
    context.dbLogicController()->initializeDatabase(context.databasePath());


    // 检查是否已登录（有有效的 Token）
    AuthService& authService = AuthService::getInstance();
    if (authService.isLoggedIn()) {
        // 如果已登录，直接显示主窗口
        MainWindow* mainWindow = MainWindow::instance();
        // 从 AuthService 获取当前登录的用户ID
        QString userId = authService.getCurrentUserId();
        if (!userId.isEmpty()) {
            mainWindow->userid = userId;
            mainWindow->requestQueryUser();
            mainWindow->show();
            Logger::getInstance().log("User already logged in, userId: " + userId);
        }
        else {
            Logger::getInstance().error("User logged in but userId is empty, showing login window");
            // 如果 userId 为空，清除登录状态，显示登录窗口
            authService.logout();
        }

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

    }
    else {


        /*DbQWidget* dbTestWidget = new DbQWidget();
        dbTestWidget->show();*/

        // 如果未登录，显示登录窗口
        LoginWindow* loginWindow = new LoginWindow();

        // 连接登录成功信号，登录成功后显示主窗口
        QObject::connect(loginWindow, &LoginWindow::loginSucceeded,
            [loginWindow]() {
                qDebug() << "main.cpp: 收到 loginSucceeded 信号";
                // 登录成功后，隐藏登录窗口，显示主窗口
                loginWindow->hide();
                qDebug() << "main.cpp: 隐藏登录窗口，准备显示主窗口";

                // 从 AuthService 获取当前登录的用户ID
                AuthService& authService = AuthService::getInstance();
                QString userId = authService.getCurrentUserId();

                if (userId.isEmpty()) {
                    Logger::getInstance().error("Login succeeded but userId is empty");
                    qDebug() << "main.cpp: 错误：登录成功但 userId 为空";
                    return;
                }

                MainWindow* mainWindow = MainWindow::instance();
                mainWindow->userid = userId;
                Logger::getInstance().log("Login succeeded, loading user data for userId: " + userId);

                // 读取用户数据
                mainWindow->requestQueryUser();
                mainWindow->show();
                //MainWindow::instance()->show();
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
