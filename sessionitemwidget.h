#ifndef SESSIONITEMWIDGET_H
#define SESSIONITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SessionItemWidget: public QWidget
{
    Q_OBJECT
public:
    explicit SessionItemWidget(QWidget *parent = nullptr);

    // 设置数据的接口
    void setAvatar(const QString &path);
    void setTitle(const QString &title);
    void setMessage(const QString &message);
    void setTime(const QString &time);

private:
    QLabel *m_avatarLabel;  // 头像
    QLabel *m_titleLabel;   // 名字
    QLabel *m_msgLabel;     // 最新消息
    QLabel *m_timeLabel;    // 时间
};

#endif // SESSIONITEMWIDGET_H
