#include "DataLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

QList<SessionInfo> DataLoader::loadFromFile(const QString& filePath)
{
    QList<SessionInfo> sessionList;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open mock data file:" << filePath;
        return sessionList;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return sessionList;
    }

    QJsonArray rootArray = doc.array();

    for (const QJsonValue& val : rootArray) {
        QJsonObject sessionObj = val.toObject();

        QString uid = sessionObj["uid"].toString();
        QString name = sessionObj["username"].toString();
        QString avatar = sessionObj["avatar"].toString();

        SessionInfo session(uid, name);
        session.setAvatarPath(avatar);

        // 解析消息
        if (sessionObj.contains("messages") && sessionObj["messages"].isArray()) {
            QJsonArray msgArray = sessionObj["messages"].toArray();
            for (const QJsonValue& msgVal : msgArray) {
                QJsonObject msgObj = msgVal.toObject();

                Message msg;
                msg.senderId = msgObj["senderId"].toString();
                msg.content = msgObj["content"].toString();
                msg.isSelf = msgObj["isSelf"].toBool();
                msg.timestamp = QDateTime::fromString(msgObj["time"].toString(), "yyyy-MM-dd HH:mm:ss");

                if (!msg.timestamp.isValid()) {
                    qDebug() << "时间解析失败! UID:" << uid
                        << "原始字符串:" << msgObj["time"].toString();
                }
                else {
                    // qDebug() << "时间解析成功:" << msg.timestamp;
                }

                session.addNewMessage(msg);
            }
        }

        if (sessionObj.contains("unread")) {
            int explicitUnread = sessionObj["unread"].toInt();
            session.setUnreadCount(explicitUnread);
        }

        sessionList.append(session);
    }

    return sessionList;
}
