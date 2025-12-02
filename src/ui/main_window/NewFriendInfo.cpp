// NewFriendInfo.cpp
/*
 * 页面名称：新朋友申请同意后弹窗
 * 功能：可修改备注等
 * 用法：通过NewFriendList唤起
 */
#include "NewFriendInfo.h"
#include <QVBoxLayout>
#include <QPixmap>

NewFriendInfo::NewFriendInfo(const QString& name,
    const QString& avatarPath,
    QWidget* parent)
    : QDialog(parent), m_name(name)
{
    this->setFixedSize(300, 380);
    //this->setWindowTitle("个人资料");

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    // 头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(100, 100);
    m_avatarLabel->setScaledContents(true);

    QPixmap pix(avatarPath);
    if (!pix.isNull())
        m_avatarLabel->setPixmap(pix);

    // 昵称
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setStyleSheet("font-size:18px; font-weight:600;");

    // 账号
    m_accountLabel = new QLabel(QStringLiteral("账号：") + name.toLower(), this);
    m_accountLabel->setStyleSheet("color:gray;");

    // 发消息按钮
    m_msgBtn = new QPushButton(QStringLiteral("发消息"), this);
    m_msgBtn->setFixedHeight(40);
    m_msgBtn->setStyleSheet(R"(
        QPushButton {
            background:#07c160;
            color:white;
            border-radius:6px;
            font-size:15px;
        }
        QPushButton:hover {
            background:#06ad56;
        }
    )");

    layout->addSpacing(20);
    layout->addWidget(m_avatarLabel);
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_accountLabel);
    layout->addStretch();
    layout->addWidget(m_msgBtn);

    // 点击"发消息"
    connect(m_msgBtn, &QPushButton::clicked, this, [=]() {
        
        this->accept();
        });
}


