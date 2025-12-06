#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "core/base_controller.h"
#include <QJsonObject>
#include "common/types.h"

/**
 * @brief 网络通信控制器
 * 
 * 运行在主线程，负责：
 * 1. 管理 NetworkWorker 的生命周期
 * 2. 提供网络操作的外部接口
 * 3. 转发网络事件到上层
 */
class NetworkController : public BaseController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject* parent = nullptr);
    ~NetworkController() override;

    bool initialize() override;
    void start() override;

    /**
     * @brief 获取单例
     */
    static NetworkController& instance();

public slots:
    /**
     * @brief 使用用户ID初始化网络服务
     * @param userId 用户ID
     */
    void initializeWithUserId(const QString& userId);

    /**
     * @brief 发送消息
     */
    void sendMessage(const LanChat::Message& message);
	void sendJsonMessage(const QJsonObject& jsonMessage);
    /**
     * @brief 发送文本消息
     */
    void sendTextMessage(const QString& text, const QString& receiverId);

	// 请求在线用户列表，触发onlineUsersUpdated信号，就算不调用这个函数，也会自动更新在线用户列表
    void getOnlineUsers();


signals:
    // 发送给 Worker 的信号
    void requestInitializeWithUserId(const QString& userId);
    void requestDisconnect();
    void requestSendMessage(const LanChat::Message& message);
	//可以通过定义type字段实现发送已读回执，发送用户状态等功能，需要自己构建QJsonObject中一定包含senderId和receiverId字段
	void requestSendJsonMessage(const QJsonObject& jsonMessage);
    void requestSendTextMessage(const QString& text, const QString& receiverId);
	void requestStopServer(); // 停止服务器模式,但是转换为中心服务器模式后，并不是所有客户端都可以暂停服务器的，所以这里其实没有内部逻辑
    void requestOnlineUsers();

    // 从 Worker 接收的信号（转发）
    void connected();
    void disconnected();
	void messageReceived(QJsonObject& message, const QString& from);
	void jsonMessageReceived(const QJsonObject& message);
    void textMessageReceived(const QString& text, const QString& from);
    void connectionStateChanged(bool isConnected);

    // 消息发送结果信号
    void messageSendSuccess(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& reason);
    
	// 在线用户列表更新信号，需要连接此信号以获取最新的在线用户列表，每五秒刷新一次
    void onlineUsersUpdated(const QStringList& userIds);

    // 🆕 Worker 状态变化转发
    void statusChanged(const QString& status);
	void errorOccurred(const QString& error);

private slots:
	void onmessageReceivedFromWorker(const LanChat::Message& message);

protected:
    QObject* createWorker() override;

private:
    void connectSignals();

    static NetworkController* s_instance;
};

#endif // NETWORK_CONTROLLER_H
