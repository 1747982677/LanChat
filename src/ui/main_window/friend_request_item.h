#ifndef FRIEND_REQUEST_ITEM_H
#define FRIEND_REQUEST_ITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>

/**
 * @brief 好友申请列表项组件
 * 功能：显示单个好友申请，包含头像、昵称、账号、验证消息和"同意"按钮
 */
class FriendRequestItem : public QWidget
{
    Q_OBJECT

public:
    explicit FriendRequestItem(const QString& requestId,
                              const QString& senderId,
                              const QString& senderNickname,
                              const QString& senderAccount,
                              const QString& avatarPath,
                              const QString& verifymsg,
                              QWidget* parent = nullptr);
    
    QString requestId() const { return m_requestId; }
    QString senderId() const { return m_senderId; }

signals:
    // 同意按钮点击
    void acceptClicked(const QString& requestId, const QString& senderId);

private slots:
    void onAcceptButtonClicked();

private:
    void setupUi();
    void setupStyle();

private:
    QString m_requestId;
    QString m_senderId;
    
    QLabel* m_avatarLabel;      // 头像
    QLabel* m_nicknameLabel;    // 昵称
    QLabel* m_accountLabel;     // 账号
    QLabel* m_verifymsgLabel;   // 验证消息
    QPushButton* m_acceptButton; // 同意按钮
};

#endif // FRIEND_REQUEST_ITEM_H

