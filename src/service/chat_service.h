#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <QObject>
#include <QString>
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
    
signals:
    void messageSent(const LanChat::Message& message);
    void messageReceived(const LanChat::Message& message);
    void errorOccurred(const QString& error);

private:
    ChatService() = default;
    ~ChatService() = default;
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;
};

#endif // CHAT_SERVICE_H
