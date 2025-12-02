// ContactList.cpp
/*
 * 页面名称：联系人列表
 * 功能：滚动窗可加载所有联系人
 * 用法：ContactItem是子组件
 */

#include "ContactList.h"
#include "ContactItem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include "main_window.h"
#include "service/chat_service.h"

//联系人占位

ContactList::ContactList(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ================= 按钮行 =================
    QWidget* btnWidget = new QWidget(this);
    QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(5);

    QPushButton* btn = new QPushButton(QStringLiteral("新朋友"), btnWidget);
    btnLayout->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, [=]() {
        MainWindow::instance()->setRightPages(MainWindow::NewFriendPage);

        });

    mainLayout->addWidget(btnWidget);

    // ================= 滚动区域 =================
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    QWidget* container = new QWidget;
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // 添加联系人信息
    QStringList names = { "张三", "李四", "王五","小六" };
    for (const QString& name : names) {
        ContactItem* item = new ContactItem(name, ":/lanchat/bubu.jpg");
        containerLayout->addWidget(item);

        // use name as temporary userId (replace with real userId from DB/service)
        QString userId = QString("user_%1").arg(name);
        item->setUserId(userId);
        m_itemsById.insert(userId, item);

        connect(item, &ContactItem::hoverEntered, this, [=](ContactItem* w) {
            //qDebug() << "鼠标悬停在：" << name;
            });

        connect(item, &ContactItem::hoverLeft, this, [=](ContactItem* w) {
            //qDebug() << "鼠标离开：" << name;
            });

        connect(item, &ContactItem::clicked, this, [this, userId](ContactItem* w) {
            // 打开对应聊天页（MainWindow 的页面切换实现）并标记为 active
            MainWindow::instance()->setRightPages(MainWindow::ChatPage);
            // 告诉 ChatService 当前正在查看该会话，并清空未读
            ChatService::getInstance().setActiveChatUserId(userId);
            ChatService::getInstance().markSessionRead(userId);
        });
    }

    containerLayout->addStretch();
    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    // 连接 ChatService 的未读更新信号
    connect(&ChatService::getInstance(), &ChatService::unreadCountChanged,
            this, [this](const QString& userId, int count){
        if (m_itemsById.contains(userId)) {
            ContactItem* it = m_itemsById.value(userId);
            it->setUnreadCount(count);
        } else {
            // 若未在当前列表中，可选择新增会话项或忽略
        }
    });
}

ContactList::~ContactList()
{}



