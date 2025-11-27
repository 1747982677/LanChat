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
    QStringList names = { "张三", "李四", "王五","小六"};
    for (const QString& name : names) {
        ContactItem* item = new ContactItem(name, ":/lanchat/bubu.jpg");
        containerLayout->addWidget(item);

        connect(item, &ContactItem::hoverEntered, this, [=](ContactItem* w) {
            //qDebug() << "鼠标悬停在：" << name;
            });

        connect(item, &ContactItem::hoverLeft, this, [=](ContactItem* w) {
            //qDebug() << "鼠标离开：" << name;
            });
    }

    containerLayout->addStretch();
    scroll->setWidget(container);
    mainLayout->addWidget(scroll);
}

ContactList::~ContactList()
{}



