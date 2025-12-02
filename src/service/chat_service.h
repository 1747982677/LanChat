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
    // ʹ�� QString �ı�����շ���ַ
    void sendMessage(const QString& content, const QString& receiverId);
    QStringList getOnlineUsers() const;
    // ��̬���������û�ˢ�¼��(ms)������ <=0 �رն�ʱˢ��
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
    // ��������Ϣ���У��������ߵ�ַ�洢�ı��б�
    QMap<QString, QStringList> m_pendingMessages;
    QTimer m_onlineRefreshTimer; // ��ʱˢ�������û�
    int m_refreshIntervalMs = 5000; // Ĭ�� 5 ��
};

#endif // CHAT_SERVICE_H
