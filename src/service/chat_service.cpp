#include "chat_service.h"
#include "utils/logger.h"
#include "utils/db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMutexLocker>
#include <QDebug>

ChatService::ChatService()
{
    // 将未读消息从数据库加载到内存缓存
    QSqlDatabase db = DatabaseManager::getInstance().database();
    if (db.isValid() && db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec("SELECT userId, unreadCount FROM chat_sessions")) {
            while (q.next()) {
                QString uid = q.value(0).toString();
                int cnt = q.value(1).toInt();
                QMutexLocker locker(&m_mutex);
                m_unreadMap[uid] = cnt;
            }
        } else {
            Logger::getInstance().log(QString("Failed to load chat_sessions: %1").arg(q.lastError().text()));
        }
    } else {
        Logger::getInstance().log("DB not open when ChatService initializing unread map");
    }
}

void ChatService::sendMessage(const LanChat::Message& message)
{
    Logger::getInstance().log("Sending message from service layer");
    // TODO: ���ﰴ��ԭ�����߼� -> �� DB -> ���緢��
    emit messageSent(message);
}

void ChatService::receiveMessage(const LanChat::Message& message)
{
    Logger::getInstance().log("Receiving message in service layer");

    // 1) 保存消息到数据库
    {
        QSqlDatabase db = DatabaseManager::getInstance().database();
        if (!db.isValid() || !db.isOpen()) {
            emit errorOccurred("Database not open when saving message");
            Logger::getInstance().log("DB not open when saving message");
        } else {
            QSqlQuery q(db);
            q.prepare("INSERT INTO messages (sender, receiver, content, timestamp, status, extra) VALUES (:sender, :receiver, :content, :timestamp, :status, :extra)");
            q.bindValue(":sender", message.senderId);
            q.bindValue(":receiver", message.receiverId);
            q.bindValue(":content", message.content);
            q.bindValue(":timestamp", static_cast<qint64>(message.timestamp));
            q.bindValue(":status", static_cast<int>(message.status));
            q.bindValue(":extra", "");
            if (!q.exec()) {
                Logger::getInstance().log(QString("Failed insert message: %1").arg(q.lastError().text()));
            }
        }
    }

    // 2) 如果不是当前活跃聊天，增加未读计数
    bool isActive = false;
    {
        QMutexLocker locker(&m_mutex);
        isActive = (!m_activeChatUserId.isEmpty() && m_activeChatUserId == message.senderId);
    }

    if (!isActive) {
        QSqlDatabase db = DatabaseManager::getInstance().database();
        if (db.isValid() && db.isOpen()) {
            QSqlQuery q(db);
            // 更新未读计数，如果不存在则插入
            q.prepare("UPDATE chat_sessions SET lastMessage=:lastMessage, lastTime=:lastTime, unreadCount = unreadCount + 1 WHERE userId = :userId");
            q.bindValue(":lastMessage", message.content);
            q.bindValue(":lastTime", static_cast<qint64>(message.timestamp));
            q.bindValue(":userId", message.senderId);
            if (!q.exec() || q.numRowsAffected() == 0) {
                QSqlQuery q2(db);
                q2.prepare("INSERT INTO chat_sessions (userId, nickname, avatarPath, lastMessage, lastTime, unreadCount) VALUES (:userId, :nickname, :avatarPath, :lastMessage, :lastTime, 1)");
                q2.bindValue(":userId", message.senderId);
                q2.bindValue(":nickname", QString());
                q2.bindValue(":avatarPath", QString());
                q2.bindValue(":lastMessage", message.content);
                q2.bindValue(":lastTime", static_cast<qint64>(message.timestamp));
                if (!q2.exec()) {
                    Logger::getInstance().log(QString("Insert session failed: %1").arg(q2.lastError().text()));
                }
            }
        }

        int newCount = 0;
        {
            QMutexLocker locker(&m_mutex);
            newCount = ++m_unreadMap[message.senderId];
        }
        Logger::getInstance().log(QString("Unread updated for %1 -> %2").arg(message.senderId).arg(newCount));
        qDebug() << "Unread updated for" << message.senderId << "->" << newCount;
        emit unreadCountChanged(message.senderId, newCount);
    } else {
        Logger::getInstance().log("Message received for active chat, emitting messageReceived");
        emit messageReceived(message);
    }
}

void ChatService::setActiveChatUserId(const QString& userId)
{
    QMutexLocker locker(&m_mutex);
    m_activeChatUserId = userId;
    Logger::getInstance().log(QString("Active chat set to %1").arg(userId));
}

void ChatService::markSessionRead(const QString& userId)
{
    Logger::getInstance().log(QString("markSessionRead called for %1").arg(userId));
    QSqlDatabase db = DatabaseManager::getInstance().database();
    if (db.isValid() && db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("UPDATE chat_sessions SET unreadCount = 0 WHERE userId = :userId");
        q.bindValue(":userId", userId);
        if (!q.exec()) {
            Logger::getInstance().log(QString("Failed clear unreadCount: %1").arg(q.lastError().text()));
        }
    }

    {
        QMutexLocker locker(&m_mutex);
        m_unreadMap[userId] = 0;
    }
    emit unreadCountChanged(userId, 0);
}

int ChatService::getUnreadCount(const QString& userId)
{
    QMutexLocker locker(&m_mutex);
    return m_unreadMap.value(userId, 0);
}
