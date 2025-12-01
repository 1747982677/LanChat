#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include "common/types.h"

class ChatService : public QObject
{
    Q_OBJECT

public:
    static ChatService& getInstance() {
        static ChatService instance;
        return instance;
    }
    
    void sendMessage(const LanChat::Message& message);
    void receiveMessage(const LanChat::Message& message);

    // 未读计数 API
    void setActiveChatUserId(const QString& userId);
    void markSessionRead(const QString& userId);
    int getUnreadCount(const QString& userId);

signals:
    void messageSent(const LanChat::Message& message);
    void messageReceived(const LanChat::Message& message);
    void unreadCountChanged(const QString& userId, int count);
    void errorOccurred(const QString& error);

private:
    ChatService();
    ~ChatService() = default;
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

    QMap<QString,int> m_unreadMap; // 内存中的未读计数缓存
    QString m_activeChatUserId;
    QMutex m_mutex;
};

#endif // CHAT_SERVICE_H
