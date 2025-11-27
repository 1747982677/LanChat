#ifndef MESSAGE_DAO_H
#define MESSAGE_DAO_H

#include "message.h"
#include <QVector>

class MessageDao {
public:
    MessageDao() = delete;

    // 插入消息，返回新 id 或 -1
    static qint64 insertMessage(const Message& msg);

    // 查询会话（按时间升序）
    static QVector<Message> getConversation(const QString& localUser, const QString& peer, int limit = 200);

    // 更新消息状态
    static bool updateStatus(qint64 id, int status);

    // 删除消息状态
    static bool delMsg(qint64 id);
};

#endif // MESSAGE_DAO_H