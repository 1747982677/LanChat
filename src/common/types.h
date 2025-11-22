#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QDateTime>
#include <cstdint>

namespace LanChat {

// User structure
struct User {
    uint64_t id;
    QString username;
    QString ipAddress;
    QString avatar;
    bool isOnline;
    QDateTime lastSeen;
    
    User() : id(0), isOnline(false) {}
};

// Message type
enum class MessageType {
    Text = 0,
    Image = 1,
    File = 2,
    System = 3
};

// Message structure
struct Message {
    uint64_t id;
    uint64_t senderId;
    uint64_t receiverId;
    MessageType type;
    QString content;
    QDateTime timestamp;
    bool isRead;
    
    Message() : id(0), senderId(0), receiverId(0), 
                type(MessageType::Text), isRead(false) {}
};

} // namespace LanChat

#endif // TYPES_H
