#ifndef NCHATVIEW_H
#define NCHATVIEW_H
#include <QListWidget>
#include <QPropertyAnimation>
#include "messagedata.h"

class NChatView: public QListWidget
{
    Q_OBJECT
public:
    explicit NChatView(QWidget *parent = nullptr);
    void addMessage(const QString &text, MsgRole role, const QString &avatarPath = "");
    void addTime(const QString &timeStr);
    void insertMessageFront(const MessageData &msg);
    void appendMessage(const MessageData &msg);

signals:
    void sigLoadingHistory();
protected:
    // 重写滚轮事件
    void wheelEvent(QWheelEvent *event) override;

private:
    // 用于平滑滚动的动画
    QPropertyAnimation *m_scrollAnimation;
    int m_targetScrollValue; // 滚动的目标位置
};

#endif // NCHATVIEW_H
