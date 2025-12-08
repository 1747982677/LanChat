#ifndef NCHATVIEW_H
#define NCHATVIEW_H

#include <QListWidget>
#include <QPropertyAnimation>
#include "SessionInfo.h"

class NChatView : public QListWidget
{
    Q_OBJECT
public:
    explicit NChatView(QWidget* parent = nullptr);

    // 设置当前会话数据
    void setSessionInfo(const SessionInfo& info);

    // 追加单条消息（发送或接收新消息时调用）
    void appendMessage(const UiMessage& msg, const QString& avatarPath);

    // 添加时间条
    void addTimeItem(const QDateTime& time);

signals:
    void sigLoadingHistory();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override; // 新增声明

private:
    QPropertyAnimation* m_scrollAnimation;
    int m_targetScrollValue;

    QString m_currentOtherAvatar;
    QDateTime m_lastMsgTime;      
};

#endif // NCHATVIEW_H
