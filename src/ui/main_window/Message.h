#ifndef UI_MESSAGE_H
#define UI_MESSAGE_H

#include <QString>
#include <QDateTime>

// UI 层消息模型，避免与模型/网络层 Message 混淆
struct UiMessage {
    QString mid;
    QString senderId;
    QString content;
    QDateTime timestamp;
    bool isSelf;
};


#endif // UI_MESSAGE_H
