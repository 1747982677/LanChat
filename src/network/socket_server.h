#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QWebSocket>
#include <QWebSocketServer>

/**
 * @brief WebSocket 中心服务器类 - 接受客户端连接并转发消息
 * 
 * 功能：
 * - 启动/停止 WebSocket 服务器
 * - 管理多个客户端连接
 * - 根据消息中的 receiverId 转发消息给目标客户端
 * - 广播消息给所有客户端
 * - 维护客户端 userId 与连接的映射关系
 */
class SocketServer : public QObject
{
    Q_OBJECT

public:
    explicit SocketServer(QObject *parent = nullptr);
    ~SocketServer();

    /**
     * @brief 启动 WebSocket 服务器
     * @param port 监听端口
     * @return 成功返回 true，失败返回 false
     */
    bool startServer(quint16 port);

    /**
     * @brief 停止服务器并断开所有客户端连接
     */
    void stopServer();

    /**
     * @brief 检查服务器是否正在运行
     * @return 运行中返回 true
     */
    bool isRunning() const;

    /**
     * @brief 获取服务器监听的端口
     * @return 端口号，未运行时返回 0
     */
    quint16 getServerPort() const;

    /**
     * @brief 向指定 userId 的客户端发送消息
     * @param userId 目标用户ID
     * @param message 消息内容
     * @return 成功返回 true
     */
    bool sendMessageToUser(const QString& userId, const QString& message);

    /**
     * @brief 向指定地址的客户端发送消息
     * @param clientAddress 客户端地址 (格式: ip:port)
     * @param message 消息内容
     */
    void sendMessageToClient(const QString& clientAddress, const QString& message);

    /**
     * @brief 向所有连接的客户端广播消息
     * @param message 消息内容
     */
    void broadcastMessage(const QString& message);

    /**
     * @brief 获取连接的客户端数量
     * @return 客户端数量
     */
    int getClientCount() const;

    /**
     * @brief 获取所有客户端的地址列表
     * @return 地址列表 (格式: ip:port)
     */
    QStringList getClientAddresses() const;

    /**
     * @brief 获取所有在线用户ID列表
     * @return 用户ID列表
     */
    QStringList getOnlineUserIds() const;

    /**
     * @brief 注册客户端的 userId
     * @param clientAddress 客户端地址
     * @param userId 用户ID
     */
    void registerClientUserId(const QString& clientAddress, const QString& userId);

    /**
     * @brief 断开指定地址的客户端
     * @param clientAddress 客户端地址
     */
    void disconnectClient(const QString& clientAddress);

signals:
    /**
     * @brief 服务器启动成功信号
     * @param port 监听端口
     */
    void serverStarted(quint16 port);

    /**
     * @brief 服务器停止信号
     */
    void serverStopped();

    /**
     * @brief 服务器错误信号
     * @param error 错误信息
     */
    void serverError(const QString& error);

    /**
     * @brief 新客户端连接信号
     * @param clientAddress 客户端地址
     */
    void clientConnected(const QString& clientAddress);

    /**
     * @brief 客户端断开连接信号
     * @param clientAddress 客户端地址
     * @param userId 用户ID（如果已注册）
     */
    void clientDisconnected(const QString& clientAddress, const QString& userId);

    /**
     * @brief 收到客户端消息信号
     * @param message 消息内容
     * @param fromAddress 发送者地址
     * @param fromUserId 发送者用户ID（如果已注册）
     */
    void messageReceived(const QString& message, const QString& fromAddress, const QString& fromUserId);

    /**
     * @brief 消息转发成功信号
     * @param messageId 消息ID
     * @param toUserId 接收者用户ID
     */
    void messageForwarded(const QString& messageId, const QString& toUserId);

    /**
     * @brief 消息转发失败信号
     * @param messageId 消息ID
     * @param toUserId 接收者用户ID
     * @param reason 失败原因
     */
    void messageForwardFailed(const QString& messageId, const QString& toUserId, const QString& reason);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientTextMessageReceived(const QString& message);

private:
    /**
     * @brief 获取客户端的地址字符串
     * @param client WebSocket 客户端指针
     * @return 地址字符串 (格式: ip:port)
     */
    QString getClientAddress(QWebSocket *client) const;

    /**
     * @brief 根据地址获取客户端指针
     * @param address 客户端地址
     * @return 客户端指针，未找到返回 nullptr
     */
    QWebSocket* getClientByAddress(const QString& address) const;

    /**
     * @brief 根据用户ID获取客户端指针
     * @param userId 用户唯一ID
     * @return 客户端指针，未找到返回 nullptr
     */
    QWebSocket* getClientByUserId(const QString& userId) const;

    QWebSocketServer *m_server;                     // WebSocket 服务器
    QList<QWebSocket*> m_clients;                   // 已连接的客户端列表
    QMap<QString, QString> m_clientAddressToUserId; // 地址 -> 用户ID 映射
    QMap<QString, QString> m_userIdToClientAddress; // 用户ID -> 地址 映射
};

#endif // SOCKET_SERVER_H
