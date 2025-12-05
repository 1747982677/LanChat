#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <QObject>
#include <QString>
#include <QWebSocket>
#include <QTimer>

/**
 * @brief WebSocket 客户端类 - 连接到中心服务器
 * 
 * 功能：
 * - 连接到唯一的中心服务器
 * - 自动重连机制（支持指数退避）
 * - 心跳检测
 * - 发送消息到服务器（由服务器转发）
 * - 接收服务器转发的消息
 */
class SocketClient : public QObject
{
    Q_OBJECT

public:
    explicit SocketClient(QObject *parent = nullptr);
    ~SocketClient();

    /**
     * @brief 连接到中心服务器
     * @param host 服务器地址
     * @param port 服务器端口
     */
    void connectToServer(const QString &host, quint16 port);

    /**
     * @brief 断开服务器连接
     */
    void disconnectFromServer();

    /**
     * @brief 向服务器发送消息（服务器将根据消息内容转发）
     * @param message 消息内容（通常是 JSON 格式，包含 receiverId）
     */
    void sendMessage(const QString &message);

    /**
     * @brief 检查是否已连接到服务器
     * @return 已连接返回 true
     */
    bool isConnected() const;

    /**
     * @brief 获取服务器地址
     * @return 服务器地址 (格式: ip:port)
     */
    QString getServerAddress() const;

    /**
     * @brief 设置最大重连尝试次数
     * @param attempts 尝试次数，0 表示无限重连
     */
    void setMaxReconnectAttempts(int attempts);

    /**
     * @brief 获取最大重连尝试次数
     * @return 尝试次数
     */
    int getMaxReconnectAttempts() const;

    /**
     * @brief 设置心跳间隔
     * @param intervalMs 心跳间隔（毫秒）
     */
    void setHeartbeatInterval(int intervalMs);

signals:
    /**
     * @brief 成功连接到服务器信号
     */
    void connected();

    /**
     * @brief 从服务器断开连接信号
     */
    void disconnected();

    /**
     * @brief 收到服务器消息信号（可能是服务器转发的其他用户消息）
     * @param message 消息内容
     */
    void messageReceived(const QString &message);

    /**
     * @brief 发生错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

    /**
     * @brief 连接状态改变信号
     * @param isConnected 是否已连接
     */
    void connectionStateChanged(bool isConnected);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError socketError);
    void onPong(quint64 elapsedTime);
    void onHeartbeatTimeout();
    void onReconnectTimeout();

private:
    /**
     * @brief 开始心跳检测
     */
    void startHeartbeat();

    /**
     * @brief 停止心跳检测
     */
    void stopHeartbeat();

    /**
     * @brief 调度重连
     */
    void scheduleReconnect();

    QWebSocket *m_socket;                   // WebSocket 连接
    QString m_serverHost;                   // 服务器主机
    quint16 m_serverPort;                   // 服务器端口
    
    // 心跳管理
    QTimer *m_heartbeatTimer;               // 心跳定时器
    int m_missedPongs;                      // 未收到的 Pong 次数
    int m_heartbeatInterval;                // 心跳间隔（毫秒）
    
    // 重连管理
    QTimer *m_reconnectTimer;               // 重连定时器
    int m_reconnectAttempts;                // 重连尝试次数
    int m_maxReconnectAttempts;             // 最大重连次数
    
    bool m_isConnected;                     // 连接状态
};

#endif // SOCKET_CLIENT_H
