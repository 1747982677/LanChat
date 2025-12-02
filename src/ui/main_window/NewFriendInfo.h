#ifndef NEWFRIENDINFO_H
#define NEWFRIENDINFO_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

class NewFriendInfo : public QDialog
{
    Q_OBJECT

public:
    explicit NewFriendInfo(const QString& name,
        const QString& avatarPath,
        QWidget* parent = nullptr);



private:
    QLabel* m_avatarLabel;//头像
    QLabel* m_nameLabel;//昵称
    QLabel* m_accountLabel;//账号
    QLineEdit* m_contactRemark;//备注


    QPushButton* m_msgBtn;//底部按钮

    QString m_name;

    void acceptNewFriendAsk();
};

#endif // NEWFRIENDINFO_H
