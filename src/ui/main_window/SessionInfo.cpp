#include "SessionInfo.h"

SessionInfo::SessionInfo(const QString& uid, const QString& username) : m_uid(uid), m_username(username) {}

void SessionInfo::addNewMessage(const Message& msg) {
    m_messages.append(msg);

    m_lastMessage = msg.content;
    m_lastTime = msg.timestamp;

    if (!msg.isSelf) {
        m_unreadCount++;
    }
}

void SessionInfo::clearUnread() {
    m_unreadCount = 0;
}
