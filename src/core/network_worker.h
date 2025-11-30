#ifndef NETWORK_WORKER_H
#define NETWORK_WORKER_H

#include "core/base_worker.h"
#include "network/socket_client.h"
#include <QJsonObject>

/**
 * @brief 网络通信 Worker
 * 
 * 运行在独立的网络线程中，负责：
 * 1. 消息收发
 * 2. 连接管理
 * 3. 心跳维护
 * 4. 网络状态监控
 */
class NetworkWorker : public BaseWorker
{
    Q_OBJECT

public:
    explicit NetworkWorker(QObject* parent = nullptr);
    ~NetworkWorker() override;

    bool initialize() override;
    void cleanup() override;

public slots:
    /**
     * @brief 连接到服务器
     * @param host 服务器地址
     * @param port 服务器端口
     */
    void connectToServer(const QString& host, quint16 port);

    /**
     * @brief 断开连接
     */
    void disconnectFromServer();

    /**
     * @brief 发送 JSON 消息
     * @param message JSON 对象
     */
    void sendMessage(const QJsonObject& message);

    /**
     * @brief 发送文本消息
     * @param text 文本内容
     */
    void sendTextMessage(const QString& text);

    /**
     * @brief 启动服务器模式
     * @param port 监听端口
     */
    void startServer(quint16 port);

    /**
     * @brief 停止服务器模式
     */
    void stopServer();

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
    void messageReceived(const QJsonObject& message, const QString& from);

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


private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketMessageReceived(const QString& message, const QString& from);
    void onSocketError(const QString& error);

private:
    SocketClient* m_socketClient;
    bool m_isConnected;
};

#endif // NETWORK_WORKER_H
