#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class NewFriendItem : public QWidget
{
    Q_OBJECT
public:
    explicit NewFriendItem(const QString& name, const QString& avatarPath, QWidget* parent = nullptr);

signals:
    void hoverEntered(NewFriendItem* item);
    void hoverLeft(NewFriendItem* item);
    void agreeClicked(NewFriendItem* item);   // 点击“同意”按钮


protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QLabel* m_avatarLabel;//头像
    QLabel* m_nameLabel;//昵称

    QString m_defaultStyle;  // 默认背景样式
    QString m_hoverStyle;    // 悬停背景样式

    
};
