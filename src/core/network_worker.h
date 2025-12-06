#ifndef NETWORK_WORKER_H
#define NETWORK_WORKER_H

#include "core/base_worker.h"
#include "service/chat_service.h"
#include <QJsonObject>
#include "common/types.h"  

/**
 * @brief 网络通信 Worker
 * 
 * 运行在独立的网络线程中，负责：
 * 1. 消息收发
 * 2. 连接管理
 * 3. 心跳维护
 * 4. 网络状态监控
 * 5. 与 ChatService 集成，处理消息转发
 */
class NetworkWorker : public BaseWorker
{
    Q_OBJECT

public:
    explicit NetworkWorker(QObject* parent = nullptr);
    ~NetworkWorker() override;

    void cleanup() override;

public slots:
    bool initialize() override;
    
    /**
     * @brief 初始化网络服务（带用户ID）
     * @param userId 用户ID，必须在调用此方法传入唯一Id
     * @return 是否初始化成功
     */
    void initializeChatService(const QString& UserId);

    /**
     * @brief 断开连接
     */
    void disconnectFromServer();

    /**
     * @brief LanChat::Message 发送 推荐使用这种方法发送消息
     * @param LanChat::Message 对象
     */
    void sendMessage(const LanChat::Message& message);

    /**
	 * @brief 发送文本消息，但需要指定接收者ID
     * @param text 文本内容
     */
    void sendTextMessage(const QString& text, const QString& reveicerId);


    /**
     * @brief 停止服务器模式
     */
    void stopServer();

	//要求更新在线用户列表，触发onlineUsersUpdated信号，就算不调用这个函数，也会自动更新在线用户列表
    void requestOnlineUsers();

signals:
    /**
     * @brief 连接成功信号
     */
    void connected();

    /**
     * @brief 断开连接信号
     */
    void disconnected();

    /**
     * @brief 收到消息信号
     * @param message JSON 消息对象
     * @param from 发送者地址
     */
    void messageReceived(const LanChat::Message& message);

    /**
     * @brief 收到文本消息信号
     */
    void textMessageReceived(const QString& text, const QString& from);

    /**
     * @brief 连接状态变化信号
     */
    void connectionStateChanged(bool isConnected);


    // 消息发送结果信号
    void messageSendSuccess(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& reason);
	// 在线用户列表更新信号
    void onlineUsersUpdated(const QStringList& userIds);


private slots:
    // ChatService 信号处理
    void onChatServiceMessageSent(const LanChat::Message& message);
    void onChatServiceMessageReceived(const LanChat::Message& message);
    void onChatServiceError(const QString& error);
    void onChatServiceOnlineUsersUpdated(const QStringList& userIds);
    

private:
    ChatService* m_chatService;  // ChatService 实例指针
    bool m_isConnected;
    QString m_currentUserId;  // 当前用户ID
};

#endif // NETWORK_WORKER_H
