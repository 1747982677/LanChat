#ifndef MESSAGEDATA_H
#define MESSAGEDATA_H

#include <QString>
#include <QDateTime>

enum class MsgRole { Self, Other, Time };

class MessageData
{
public:
    MessageData(const QString &text, MsgRole role, const QString &avatarurl = "")
        : m_content(text), m_role(role), m_avatarurl(avatarurl)
    {
        m_timestamp = QDateTime::currentMSecsSinceEpoch();
        m_msgId = QString::number(m_timestamp);
    }

    // 简单的 Getter
    QString content() const { return m_content; }
    MsgRole role() const { return m_role; }
    QString avatarurl() const { return m_avatarurl; }
    qint64 timestamp() const { return m_timestamp; }

private:
    QString m_content;
    MsgRole m_role;
    qint64 m_timestamp;
    QString m_avatarurl;   // 头像路径
    QString m_msgId;      // 消息唯一ID (用于更新状态)
    QString senderId;   // 发送者ID
};

#endif // MESSAGEDATA_H
