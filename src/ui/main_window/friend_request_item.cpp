#include "friend_request_item.h"
#include <QPixmap>
#include <QVBoxLayout>

FriendRequestItem::FriendRequestItem(const QString& requestId,
                                     const QString& senderId,
                                     const QString& senderNickname,
                                     const QString& senderAccount,
                                     const QString& avatarPath,
                                     const QString& verifymsg,
                                     QWidget* parent)
    : QWidget(parent)
    , m_requestId(requestId)
    , m_senderId(senderId)
{
    setupUi();
    setupStyle();
    
    // 设置数据
    QString finalAvatarPath = avatarPath;
    
    // 如果头像路径为空，使用默认头像
    if (finalAvatarPath.isEmpty()) {
        finalAvatarPath = ":/lanchat/bubu.jpg";
    }
    
    QPixmap pixmap(finalAvatarPath);
    if (!pixmap.isNull()) {
        m_avatarLabel->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 如果默认头像也加载失败，尝试加载默认头像路径再试一次
        QPixmap defaultPixmap(":/lanchat/bubu.jpg");
        if (!defaultPixmap.isNull()) {
            m_avatarLabel->setPixmap(defaultPixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // 如果默认头像也加载失败，显示文字提示
            m_avatarLabel->setText("头像");
        }
    }
    
    m_nicknameLabel->setText(senderNickname.isEmpty() ? "未设置昵称" : senderNickname);
    m_accountLabel->setText(senderAccount);
    m_verifymsgLabel->setText(verifymsg.isEmpty() ? "请求添加你为好友" : verifymsg);
}

void FriendRequestItem::setupUi()
{
    setFixedHeight(80);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(12);
    mainLayout->setAlignment(Qt::AlignTop);  // 设置布局顶部对齐
    
    // 头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(50, 50);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet("border: 1px solid #e0e0e0; border-radius: 4px; background-color: #f5f5f5;");
    mainLayout->addWidget(m_avatarLabel, 0, Qt::AlignTop);  // 头像顶部对齐
    
    // 中间信息区域
    QWidget* infoWidget = new QWidget(this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(4);
    infoLayout->setAlignment(Qt::AlignTop);  // 信息区域顶部对齐
    
    m_nicknameLabel = new QLabel(infoWidget);
    m_nicknameLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #333;");
    infoLayout->addWidget(m_nicknameLabel);
    
    m_accountLabel = new QLabel(infoWidget);
    m_accountLabel->setStyleSheet("font-size: 13px; color: #999;");
    infoLayout->addWidget(m_accountLabel);
    
    m_verifymsgLabel = new QLabel(infoWidget);
    m_verifymsgLabel->setStyleSheet("font-size: 12px; color: #666;");
    m_verifymsgLabel->setWordWrap(true);
    infoLayout->addWidget(m_verifymsgLabel);
    
    // 移除 addStretch()，让信息置顶显示
    mainLayout->addWidget(infoWidget, 1, Qt::AlignTop);  // 信息区域顶部对齐
    
    // 同意按钮
    m_acceptButton = new QPushButton("同意", this);
    m_acceptButton->setFixedSize(70, 35);
    m_acceptButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #07c160;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #06ad56;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #059648;"
        "}"
    );
    connect(m_acceptButton, &QPushButton::clicked, this, &FriendRequestItem::onAcceptButtonClicked);
    mainLayout->addWidget(m_acceptButton, 0, Qt::AlignTop);  // 按钮顶部对齐
}

void FriendRequestItem::setupStyle()
{
    setStyleSheet(
        "FriendRequestItem {"
        "   background-color: white;"
        "   border-bottom: 1px solid #e0e0e0;"
        "}"
        "FriendRequestItem:hover {"
        "   background-color: #f5f5f5;"
        "}"
    );
}

void FriendRequestItem::onAcceptButtonClicked()
{
    emit acceptClicked(m_requestId, m_senderId);
}

