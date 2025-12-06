#ifndef CHATTITLE_H
#define CHATTITLE_H

#include <QWidget>
#include <QLabel>
#include "SessionInfo.h"

class ChatTitle : public QWidget
{
    Q_OBJECT
public:
    explicit ChatTitle(QWidget* parent = nullptr);
    void setSessionInfo(const SessionInfo& info);

private:
    QLabel* m_nameLabel;
};

#endif // CHATTITLE_H
