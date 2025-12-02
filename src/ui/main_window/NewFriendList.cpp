// NewFriendList.cpp
/*
 * 页面名称：新朋友申请列表
 * 功能：滚动窗可加载所有新朋友申请
 * 用法：NewFriendItem是子组件
 */
#include "NewFriendList.h"
#include "NewFriendItem.h"
#include "NewFriendInfo.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QDebug>

NewFriendList::NewFriendList(QWidget* parent)
    : QWidget(parent)
{
    // 1. 创建滚动区域和容器
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);

    m_container = new QWidget;
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(0);
    m_containerLayout->addStretch(); // 保证最后有伸缩空间

    m_scroll->setWidget(m_container);

    // 2. 设置页面主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scroll);
    setLayout(mainLayout);

    // 3. 加载数据
    loadData();
}

NewFriendList::~NewFriendList()
{
}

void NewFriendList::loadData()
{
    // 清空旧的数据（如果有的话）
    QLayoutItem* child;
    while ((child = m_containerLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    // 示例数据
    QStringList names = {
        QStringLiteral("zzz"),
        QStringLiteral("axin"),
        QStringLiteral("困困包"),
        QStringLiteral("一口一个小笼包")
    };

    for (const QString& name : names) {
        NewFriendItem* item = new NewFriendItem(name, ":/lanchat/bubu.jpg", this);

        m_containerLayout->addWidget(item);

        // 鼠标悬停信号（可以注释或调试用）
        connect(item, &NewFriendItem::hoverEntered, this, [=](NewFriendItem*) {
            //qDebug() << "鼠标悬停在：" << name;
            });
        connect(item, &NewFriendItem::hoverLeft, this, [=](NewFriendItem*) {
            //qDebug() << "鼠标离开：" << name;
            });

        // 点击按钮 → 打开资料弹窗 → 更新数据
        connect(item, &NewFriendItem::agreeClicked, this, [=](NewFriendItem* w) {
            NewFriendInfo* info = new NewFriendInfo(name, ":/lanchat/bubu.jpg", this);
            info->show();

            qDebug() << "按钮按下：" << name;

            // 更新数据
            acceptNewFriendAsk(name);
            });
    }

    m_containerLayout->addStretch();
}
void NewFriendList::acceptNewFriendAsk(const QString& name)
{
    // 处理逻辑，比如刷新列表、删除条目、打印日志等
    qDebug() << "接受好友请求，需要调用更新数据函数：" << name;

    // TODO: 根据实际业务，删除对应条目或刷新列表
   
}
