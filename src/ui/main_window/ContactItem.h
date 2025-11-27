#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class ContactItem : public QWidget
{
    Q_OBJECT
public:
    explicit ContactItem(const QString& name, const QString& avatarPath, QWidget* parent = nullptr);

signals:
    void hoverEntered(ContactItem* item);
    void hoverLeft(ContactItem* item);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QLabel* m_avatarLabel;//头像
    QLabel* m_nameLabel;//昵称

    QString m_defaultStyle;  // 默认背景样式
    QString m_hoverStyle;    // 悬停背景样式
};
