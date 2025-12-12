#ifndef SESSIONINFO_H
#define SESSIONINFO_H

#include "Message.h"
#include <QString>
#include <QDateTime>
#include <QList>


class SessionInfo
{
public:
    SessionInfo() {};
    SessionInfo(const QString& uid, const QString& username);

    // getter
    bool isTop() const { return m_isTop; }
    QString uid() const { return m_uid; }
    QString username() const { return m_username; }
    QString avatarPath() const { return m_avatarPath; }
    QString lastMessage() const { return m_lastMessage; }
    QDateTime lastTime() const { return m_lastTime; }
    int unreadCount() const { return m_unreadCount; }
    const QList<UiMessage>& messages() const { return m_messages; }

    // setter
    void setTop(const bool isTop) { m_isTop = isTop; }
    void setUid(const QString& uid) { m_uid = uid; }
    void setUsername(const QString& username) { m_username = username; }
    void setAvatarPath(const QString& path) { m_avatarPath = path; }
    void setLastMessage(const QString& msg) { m_lastMessage = msg; }
    void setLastTime(const QDateTime& time) { m_lastTime = time; }
    void setUnreadCount(int count) { m_unreadCount = count; }

    void addNewMessage(const UiMessage& msg);
    void clearUnread();

private:
    bool m_isTop = false;
    QString m_uid;
    QString m_username;
    QString m_avatarPath;
    QString m_lastMessage;
    QDateTime m_lastTime;
    int m_unreadCount = 0;
    QList<UiMessage> m_messages;
};

#endif // SESSIONINFO_H
