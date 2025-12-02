// NewFriendItem.cpp
/*
 * 页面名称：新朋友申请列表组件
 * 功能：显示每个单独的新朋友申请
 * 用法：在NewFriendList集成
 */
#include <QHBoxLayout>
#include <QPixmap>
#include<QPushButton>
#include <QStyle>
#include "NewFriendItem.h"

NewFriendItem::NewFriendItem(const QString& name, const QString& avatarPath, QWidget* parent)
    : QWidget(parent)
{
    this->setStyleSheet(R"(
    NewFriendItem {
        background-color: transparent;
        border-radius:4px;
    }

    NewFriendItem[hover="true"] {
        background-color: #e0e0e0;
    }
)");

    this->setFixedHeight(70);
    this->setAttribute(Qt::WA_StyledBackground, true); 

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(8);

    //头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(40, 40);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setStyleSheet("background: transparent;");

    QPixmap pixmap(avatarPath);
    if (!pixmap.isNull()) {
        m_avatarLabel->setPixmap(pixmap);
    }
    else {
        m_avatarLabel->setStyleSheet("background-color:#666;");
    }
    //昵称
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setStyleSheet("background: transparent;");

    QLabel* askBeFriend = new QLabel(QStringLiteral("请求加为好友"), this);
    askBeFriend->setStyleSheet("background: transparent;");
   
    //同意按钮
    QPushButton* askBeFriendBtn = new QPushButton(QStringLiteral("同意"), this);
    askBeFriendBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #07c160;
                color: white;
                border-radius: 6px;
                padding: 4px 10px;
            }
            QPushButton:hover {
                background-color: #06ad56;
            }
        )");

    connect(askBeFriendBtn, &QPushButton::clicked, this, [=]() {
        emit agreeClicked(this);
        });


    layout->addWidget(m_avatarLabel, 1);
    layout->addWidget(m_nameLabel, 2);
  
    layout->addWidget(askBeFriend, 1);
    layout->addStretch(1);
    layout->addWidget(askBeFriendBtn, 1);


    m_defaultStyle = "background-color: transparent; border-radius: 4px;";
    m_hoverStyle = "background-color: #e0e0e0; border-radius: 4px;";
}



void NewFriendItem::enterEvent(QEnterEvent* event)
{
    this->setProperty("hover", true);
    style()->polish(this);
    emit hoverEntered(this);
    QWidget::enterEvent(event);
}

void NewFriendItem::leaveEvent(QEvent* event)
{
    this->setProperty("hover", false);
    style()->polish(this);
    emit hoverLeft(this);
    QWidget::leaveEvent(event);
}

