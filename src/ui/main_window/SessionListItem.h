#ifndef SESSIONLISTITEM_H
#define SESSIONLISTITEM_H
#include "SessionInfo.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SessionListItem : public QWidget
{
    Q_OBJECT
public:
    explicit SessionListItem(QWidget* parent = nullptr);

    void setData(const SessionInfo& data);
    SessionInfo getData() const { return m_data; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override; // 新增声明

    // 内部辅助函数
private:
    QString formatTime(const QDateTime& date);
    QString elideText(const QString& text, int width);
    void updateMessageLabel();

private:
    SessionInfo m_data; // 保存一份数据副本或指针

    QLabel* m_avatarLabel;
    QLabel* m_titleLabel;
    QLabel* m_msgLabel;
    QLabel* m_timeLabel;
    QLabel* m_unReadNum;
};



#endif // SESSIONLISTITEM_H
