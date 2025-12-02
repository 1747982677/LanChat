#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QTimer>
#include <network/socket_client.h>

class ChatService : public QObject
{
    Q_OBJECT

public:
    static ChatService& getInstance() {
        static ChatService instance;
        return instance;
    }
    void Init(quint16 serverPort);
    // 使用 QString 文本与接收方地址
    void sendMessage(const QString& content, const QString& receiverId);
    QStringList getOnlineUsers() const;
    // 动态调整在线用户刷新间隔(ms)，传入 <=0 关闭定时刷新
    void setOnlineRefreshInterval(int intervalMs);

signals:
    void messageSent(const QString& content, const QString& toAddress);
    void messageReceived(const QString& content, const QString& from);
    void errorOccurred(const QString& error);
    void onlineUsersUpdated(const QStringList& onlineUsers);

private slots:
    void onSocketMessageReceived(const QString& message, const QString& from);
    void onSocketError(const QString& error);
    void onOnlineAddressesReceived(const QStringList& addresses);
    void onConnectedToServer(const QString& address);
    void onRefreshOnlineUsers();

private:
    ChatService();
    ~ChatService() = default;
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

    SocketClient* m_socketClient = nullptr;
    QStringList m_onlineUsers;
    // 待发送消息队列：按接收者地址存储文本列表
    QMap<QString, QStringList> m_pendingMessages;
    QTimer m_onlineRefreshTimer; // 定时刷新在线用户
    int m_refreshIntervalMs = 5000; // 默认 5 秒
};

#endif // CHAT_SERVICE_H
