#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUdpSocket>
#include "common/types.h"
#include <network/socket_client.h>
#include <network/socket_server.h>

class ChatService : public QObject
{
    Q_OBJECT

public:
    static ChatService& getInstance() {
        static ChatService instance;
        return instance;
    }
    
    /**
     * @brief 初始化 ChatService（必须在第一次使用前调用）
     * @param myUserId 本地用户ID
     * @return 成功返回 true
     * 
     * 注意：此方法必须在调用其他方法之前调用一次
     */
    bool initialize(const QString& myUserId);
    bool isInitialized() const { return m_isInitialized; }

    /**
    * @brief 兼容旧接口 - 初始化并自动判断是否为第一个客户端
    * @param port 端口号（8080表示作为服务器，其他表示作为客户端连接到localhost:8080）
    * @param myUserId 本地用户ID
    */
    void Init(quint16 port, const QString& myUserId);

    /**
     * @brief 自动初始化 - 检测局域网内是否有服务器，自动决定模式
     * @param serverPort 服务器端口（默认8080）
     * @param timeoutMs 检测超时时间（毫秒）
     * @return 成功返回 true
     * 
     * 功能：
     * 1. 发送 UDP 广播检测是否有服务器
     * 2. 如果检测到服务器，作为客户端连接
     * 3. 如果没有检测到，启动服务器并连接到自己
     */
    bool autoInit(quint16 serverPort = 8080, int timeoutMs = 1000);
    
    //初始化为服务器模式（第一个客户端）
    bool initAsServer(quint16 serverPort = 8080);
    
    // 初始化为客户端模式（后续客户端）
    bool initAsClient(const QString& serverHost, quint16 serverPort);
    
	//发送消息（通过中心服务器转发）
    void sendMessage(const LanChat::Message& message);
	//发送Jason对象消息（通过中心服务器转发，通过自己构建的QJsonObject 可以实现发送已读回执，发送用户状态等功能）
	void sendJsonMessage(const QJsonObject& jsonMessage);
    QStringList getOnlineUsers() const;
    QString getMyUserId() const { return m_myUserId; }
    void setOnlineRefreshInterval(int intervalMs);
    
    //注册在线用户（本地缓存）
    void registerUser(const QString& userId, const QString& address = QString());
    
    //获取用户地址（在服务器模式下不使用）
    QString getUserAddress(const QString& userId) const;
    
    //获取地址对应的用户ID（在服务器模式下不使用）
    QString getAddressUserId(const QString& address) const;

     //请求服务器返回在线用户列表
    void requestOnlineUsers();

    //检查是否为服务器模式
    bool isServerMode() const { return m_isServerMode; }

signals:
     // 消息发送信号（本地触发，不等待服务器确认）
    void messageSent(const LanChat::Message& message);
	void JsonMessageSent(const QJsonObject& jsonMessage);
    
    //收到消息信号
    void messageReceived(const LanChat::Message& message);
	void JsonMessageReceived(const QJsonObject& jsonMessage);
    //错误发生信号
    void errorOccurred(const QString& error);
    
    //在线用户列表更新信号
    void onlineUsersUpdated(const QStringList& userIds);
    
    //服务器发现结果信号
    void serverDiscovered(bool found, const QString& serverAddress);

private slots:
    void onSocketMessageReceived(const QString& message);
    void onRefreshOnlineUsers();
    void onDiscoveryTimeout();
    void onDiscoveryResponse();

private:
    ChatService();
    ~ChatService() = default;
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

    //向服务器注册自己的 userId
    void registerToServer();

    //生成临时用户ID（在服务器模式下不使用）
    QString generateTempUserId(const QString& address);
    
    //发送 UDP 广播检测服务器
    void discoverServer(quint16 port, int timeoutMs);

    SocketServer* m_socketServer = nullptr;      // 服务器（仅服务器模式）
    SocketClient* m_socketClient = nullptr;      // 连接到中心服务器的客户端
    
    QTimer m_onlineRefreshTimer;                 // 定时刷新在线用户
    int m_refreshIntervalMs = 5000;              // 默认 5 秒
    
    QStringList m_onlineUserIds;                 // 在线用户ID列表
    
    QString m_myUserId;                          // 本机用户ID（在 initialize() 中设置）
    QString m_serverHost;                        // 服务器地址
    quint16 m_serverPort = 0;                    // 服务器端口
    bool m_isServerMode = false;                 // 是否为服务器模式
    bool m_isInitialized = false;                // 是否已初始化
    
    // UDP 服务器发现
    QUdpSocket* m_discoverySocket = nullptr;     // UDP 发现套接字
    QTimer* m_discoveryTimer = nullptr;          // 发现超时定时器
    quint16 m_discoveryPort = 45454;             // 发现端口
    QString m_discoveredServerAddress;           // 发现的服务器地址
    bool m_serverFound = false;                  // 是否找到服务器
};

#endif // CHAT_SERVICE_H