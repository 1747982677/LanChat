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
    
    // 设置用户状态（在线/离线）
    void setUserStatus(bool isOnline);

signals:
    void hoverEntered(ContactItem* item);
    void hoverLeft(ContactItem* item);
    void clicked(ContactItem* item);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* m_avatarLabel;//ͷ��
    QLabel* m_nameLabel;//�ǳ�
    QLabel* m_statusIndicator;  // 状态指示器（小圆点）
    QLabel* m_badgeLabel; // δ���Ǳ�

    QString m_defaultStyle;  // Ĭ�ϱ�����ʽ
    QString m_hoverStyle;    // ��ͣ������ʽ
    QString m_userId;
};
