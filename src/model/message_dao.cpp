#include "message_dao.h"
#include "../utils/db_manager.h"
#include "../utils/logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QThread>

qint64 MessageDao::insertMessage(const Message& msg)
{
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) {
        Logger::getInstance().error("DB not open in MessageDao::insertMessage");
        return -1;
    }

    QSqlQuery q(db);
    q.prepare("INSERT INTO messages (sender, receiver, content, timestamp, status, extra) VALUES (:sender, :receiver, :content, :timestamp, :status, :extra)");
    q.bindValue(":sender", msg.sender);
    q.bindValue(":receiver", msg.receiver);
    q.bindValue(":content", msg.content);
    q.bindValue(":timestamp", msg.timestamp.toSecsSinceEpoch());
    q.bindValue(":status", msg.status);
    q.bindValue(":extra", msg.extra);

    if (!q.exec()) {
        Logger::getInstance().error("Insert message failed: " + q.lastError().text());
        return -1;
    }
    return q.lastInsertId().toLongLong();
}


QVector<Message> MessageDao::getConversation(const QString& localUser, const QString& peer, int limit)
{
    quintptr id = reinterpret_cast<quintptr>(QThread::currentThreadId());
    Logger::getInstance().log(QString("当前数据库线程：lanchat_conn_%1").arg(id));

    QVector<Message> out;
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) return out;

    QSqlQuery q(db);
    q.prepare(R"(
        SELECT id, sender, receiver, content, timestamp, status, extra
        FROM messages
        WHERE (sender = :a AND receiver = :b) OR (sender = :b AND receiver = :a)
        ORDER BY timestamp ASC
        LIMIT :limit
    )");
    q.bindValue(":a", localUser);
    q.bindValue(":b", peer);
    q.bindValue(":limit", limit);

    if (!q.exec()) {
        Logger::getInstance().error("Query conversation failed: " + q.lastError().text());
        return out;
    }

    while (q.next()) {
        Message m;
        m.id = q.value("id").toLongLong();
        m.sender = q.value("sender").toString();
        m.receiver = q.value("receiver").toString();
        m.content = q.value("content").toString();
        m.timestamp = QDateTime::fromSecsSinceEpoch(q.value("timestamp").toLongLong(), Qt::UTC);
        m.status = q.value("status").toInt();
        m.extra = q.value("extra").toString();
        out.append(m);
    }
    return out;
}

bool MessageDao::updateStatus(qint64 id, int status)
{
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare("UPDATE messages SET status = :status WHERE id = :id");
    q.bindValue(":status", status);
    q.bindValue(":id", id);

    if (!q.exec()) {
        Logger::getInstance().error("Update status failed: " + q.lastError().text());
        return false;
    }
    return true;
}

bool MessageDao::delMsg(qint64 id)
{
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) return false;
    db.transaction();  // 开启事务
    QSqlQuery q(db);
    q.prepare("DELETE from messages WHERE id = :id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        Logger::getInstance().error("DELETE messages failed: " + q.lastError().text());
        db.rollback();  // 失败回滚
        return false;
    }
    db.commit();  // 成功提交
    return true;
}