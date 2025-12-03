#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

struct Message {
    qint64 id = 0;
    QString sender;
    QString receiver;
    QString content;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
    int status = 0; // 0=Sending,1=Sent,2=Received,3=Failed
    QString extra;
};

#endif // MESSAGE_H