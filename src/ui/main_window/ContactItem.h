#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class ContactItem : public QWidget
{
    Q_OBJECT
public:
    explicit ContactItem(const QString& name, const QString& avatarPath, QWidget* parent = nullptr);

    void setUserId(const QString& id) { m_userId = id; }
    QString userId() const { return m_userId; }

    void setUnreadCount(int count);

signals:
    void hoverEntered(ContactItem* item);
    void hoverLeft(ContactItem* item);
    void clicked(ContactItem* item);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_avatarLabel;//Í·Ïñ
    QLabel* m_nameLabel;//êÇ³Æ
    QLabel* m_badgeLabel; // Î´¶Á½Ç±ê

    QString m_defaultStyle;  // Ä¬ÈÏ±³¾°ÑùÊ½
    QString m_hoverStyle;    // ÐüÍ£±³¾°ÑùÊ½
    QString m_userId;
};
