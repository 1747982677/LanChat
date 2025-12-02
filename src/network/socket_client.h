#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QUdpSocket>
#include <QJsonArray>

// - 支持作为 WebSocket 服务器（接收多个客户端连接、广播与单发）
// - 支持作为 WebSocket 客户端，可以同时连接到多个远端服务器
// - 通过信号将收到的消息/错误通知上层
class SocketClient : public QObject
{
    Q_OBJECT

public:
    explicit SocketClient(QObject *parent = nullptr);
    ~SocketClient();

    // -------------------- 客户端 API --------------------
    // 发起到一台服务器的连接（会为该连接创建一个独立 QWebSocket）
    void connectToHost(const QString &host, quint16 port);
    // 断开并释放所有作为客户端的连接
    void disconnect();
    // 向指定索引的服务器发送消息
    void sendMessageToServer(int index, const QString &message);
    // 向指定地址的服务器发送消息（地址格式 ip:port）
    void sendMessageToServerByAddress(const QString &address, const QString &message);
    // 是否至少有一个已连接的客户端
    bool isConnected() const;
    // 获取客户端连接数
    int getClientCount() const;
    // 获取客户端地址列表
    QStringList getClientAddresses() const;

    // allow configuring max reconnect attempts
    void setMaxReconnectAttempts(int attempts) { maxReconnectAttempts = attempts; }
    int getMaxReconnectAttempts() const { return maxReconnectAttempts; }

    // -------------------- 服务器 API --------------------
    // 启动/停止本机 WebSocket 服务器
    bool startServer(quint16 port);
    void stopServer();
    bool isServerRunning() const;
    quint16 getServerPort() const;
    // 向所有连接到本服务器的客户端广播消息
    void broadcastMessage(const QString &message);
    // 向指定接入客户端发送消息（按接入索引）
    void sendMessageToClient(int index, const QString &message);
    // 向指定接入客户端发送消息（按地址 ip:port）
    void sendMessageToClientByAddress(const QString &clientAddress, const QString &message);
    // 单发：按接入索引或按地址发送
    int getConnectedClientCount() const;
    QStringList getServerClientAddresses() const;

    // -------------------- �㲥��ȡ�����û��������� --------------------
    // ͨ�����ѽ����� websocket �����Ϲ㲥��ѯ���ռ��������߽ڵ���Ӧ�ĵ�ַ��ȥ�أ�
    // timeoutMs���ȴ���Ӧ�ĺ���������ʱ��ᴥ�� onlineAddressesReceived
    void broadcastGetOnlineAddresses(int timeoutMs = 1000);

signals:
    // 收到消息（无论来自服务端还是客户端连接）
    // 参数：消息内容, 来源地址（格式 ip:port）
    void messageReceived(const QString &message, const QString &from);
    // 错误通用信号
    void errorOccurred(const QString &error);

    // 客户端相关
    void connected();
    void disconnected();
    void connectedToServer(const QString &address);
    void disconnectedFromServer(const QString &address);

    // 服务器相关
    void serverStarted(quint16 port);
    void serverStopped();
    void serverError(const QString &error);
    void clientConnected(const QString &clientAddress);
    void clientDisconnected(const QString &clientAddress);

    // �㲥��ѯ�������ߵ�ַ��ȥ�غ󷵻أ�
    void onlineAddressesReceived(const QStringList &addresses);

private slots:
    // 客户端槽（多个 QWebSocket 复用这些槽）
    void onClientConnected();
    void onClientDisconnected();
    void onClientTextMessageReceived(const QString &message);
    void onClientPong(quint64 elapsedTime);
    void onClientError(QAbstractSocket::SocketError socketError);

    // 服务器槽
    void onNewConnection();
    void onServerClientDisconnected();
    void onServerClientTextMessageReceived(const QString &message);

    // UDP discovery
    void onUdpReadyRead();

private:
    // 作为客户端的多个连接
    QList<QWebSocket*> clientSockets;
    // per-client maps
    QMap<QWebSocket*, QString> clientHost;
    QMap<QWebSocket*, quint16> clientPort;
	//保存心跳计时器
    QMap<QWebSocket*, QTimer*> heartbeatTimers;
	//计数器到期后进行重连，涉及到一个指数退避机制
    QMap<QWebSocket*, QTimer*> reconnectTimers;
    // 记录重连尝试次数
    QMap<QWebSocket*, int> reconnectAttempts;
	// 记录丢失的 Pong 消息数，当大于3是会
    QMap<QWebSocket*, int> missedPongs;

    // 最大重连次数，默认设置为10
    int maxReconnectAttempts;

    // 作为服务器时的监听对象及接入的客户端
    QWebSocketServer *webSocketServer;
    QList<QWebSocket*> serverClients;

	// 获取客户端地址字符串
    QString getClientAddress(QWebSocket *client) const;
	// 清理某个客户端连接资源
    void cleanupClientResources(QWebSocket* client);

    // -------------------- �㲥��ѯ���˽�г�Ա --------------------
    QTimer *onlineQueryTimer = nullptr;
    QSet<QString> onlineQueryResponses;

    // UDP discovery socket
    QUdpSocket *discoverySocket = nullptr;
    quint16 discoveryPort = 45454; // default discovery port

    // �ռ��������õĶ������/���Ӷ˵� (ip:port)�������ڷ��ֽ׶�������������
    QStringList collectLocalEndpoints() const;
};

#endif // SOCKET_CLIENT_H
