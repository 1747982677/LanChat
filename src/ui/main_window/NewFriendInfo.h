#ifndef NEWFRIENDINFO_H
#define NEWFRIENDINFO_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class NewFriendInfo : public QDialog
{
    Q_OBJECT

public:
    explicit NewFriendInfo(const QString& name,
        const QString& avatarPath,
        QWidget* parent = nullptr);



private:
    QLabel* m_avatarLabel;
    QLabel* m_nameLabel;
    QLabel* m_accountLabel;
    QPushButton* m_msgBtn;

    QString m_name;

    void acceptNewFriendAsk();
};

#endif // NEWFRIENDINFO_H
