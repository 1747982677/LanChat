#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

struct Message {
    QString mid;
    QString senderId;
    QString content;
    QDateTime timestamp;
    bool isSelf;
};


#endif // MESSAGE_H
