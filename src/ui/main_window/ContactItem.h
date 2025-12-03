#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class ContactPop;
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
    QLabel* m_avatarLabel;//Í·Ïñ
    QLabel* m_nameLabel;//êÇ³Æ
    QString m_name;//êÇ³Æ

    QString m_defaultStyle;  // Ä¬ÈÏ±³¾°ÑùÊ½
    QString m_hoverStyle;    // ÐüÍ£±³¾°ÑùÊ½

    ContactPop* m_tooltip = nullptr;
};
