#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "core/base_controller.h"
#include <QJsonObject>

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
     * @brief 连接到服务器
     */
    void connectToServer(const QString& host, quint16 port);

    /**
     * @brief 断开连接
     */
    void disconnectFromServer();

    /**
     * @brief 发送消息
     */
    void sendMessage(const QJsonObject& message);

    /**
     * @brief 发送文本消息
     */
    void sendTextMessage(const QString& text);

    /**
     * @brief 启动服务器
     */
    void startServer(quint16 port);

    /**
     * @brief 停止服务器
     */
    void stopServer();

signals:
    // 发送给 Worker 的信号
    void requestConnect(const QString& host, quint16 port);
    void requestDisconnect();
    void requestSendMessage(const QJsonObject& message);
    void requestSendTextMessage(const QString& text);
    void requestStartServer(quint16 port);
    void requestStopServer();

    // 从 Worker 接收的信号（转发）
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& message, const QString& from);
    void textMessageReceived(const QString& text, const QString& from);
    void connectionStateChanged(bool isConnected);

    // 消息发送结果信号
    void messageSendSuccess(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& reason);

protected:
    QObject* createWorker() override;

private:
    void connectSignals();

    static NetworkController* s_instance;
};

#endif // NETWORK_CONTROLLER_H
