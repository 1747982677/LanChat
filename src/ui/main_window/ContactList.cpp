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
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include "main_window.h"
#include "core/app_context.h"
#include "core/dblogic_controller.h"
#include "service/auth_service.h"
#include "service/chat_service.h"

ContactList::ContactList(QWidget* parent)
    : QWidget(parent)
    , m_container(nullptr)
    , m_containerLayout(nullptr)
    , m_scrollArea(nullptr)
{
    setupUi();
    
    // 连接数据库控制器信号
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    connect(dbCtrl, &DbLogicController::contactListLoaded,
            this, &ContactList::onContactListLoaded);
}

void ContactList::setupUi()
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
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_container = new QWidget;
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(0);
    m_containerLayout->addStretch();

    m_scrollArea->setWidget(m_container);
    mainLayout->addWidget(m_scrollArea);
}

void ContactList::loadContacts()
{
    // 获取当前登录用户ID
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    
    qDebug() << "=== ContactList::loadContacts() ===";
    qDebug() << "Current logged in userId:" << currentUserId;
    
    if (currentUserId.isEmpty()) {
        qDebug() << "ContactList: User not logged in, cannot load contacts";
        return;
    }
    
    // 调用数据库控制器加载联系人列表
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    qDebug() << "Calling dbCtrl->loadContactList with userId:" << currentUserId;
    dbCtrl->loadContactList(currentUserId);
}

void ContactList::onContactListLoaded(const QJsonArray& contacts)
{
    // 保存所有联系人数据（用于搜索）
    m_allContacts = contacts;
    
    // 显示联系人列表
    displayContacts(contacts);
}

void ContactList::displayContacts(const QJsonArray& contacts)
{
    // 清空现有联系人
    clearContacts();
    
    if (contacts.isEmpty()) {
        // 显示空状态提示
        QLabel* emptyLabel = new QLabel("暂无联系人", m_container);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #999; font-size: 14px; padding: 50px;");
        m_containerLayout->insertWidget(0, emptyLabel);
        return;
    }
    
    // 添加联系人项
    for (int i = 0; i < contacts.size(); ++i) {
        QJsonObject contactObj = contacts[i].toObject();
        QString friendId = contactObj["friendId"].toString();
        QString displayName = contactObj["displayName"].toString();
        QString nickname = contactObj["nickname"].toString();
        QString avatarPath = contactObj["avatarPath"].toString();
        
        // 如果头像路径为空，使用默认头像
        if (avatarPath.isEmpty()) {
            avatarPath = ":/lanchat/bubu.jpg";
        }
        
        ContactItem* item = new ContactItem(displayName, avatarPath, m_container);
        item->setUserId(friendId);
        
        // 设置用户状态（从数据中读取status字段，1表示在线，0表示离线）
        int status = contactObj["status"].toInt();
        bool isOnline = (status == 1);  // UserStatus::Online = 1
        item->setUserStatus(isOnline);
        
        m_itemsById.insert(friendId, item);
        
        connect(item, &ContactItem::hoverEntered, this, [=](ContactItem* w) {
            // 可以在这里添加悬停效果
        });
        
        connect(item, &ContactItem::hoverLeft, this, [=](ContactItem* w) {
            // 可以在这里移除悬停效果
        });
        
        connect(item, &ContactItem::clicked, this, [this, friendId, displayName](ContactItem* w) {
            // 打开对应聊天页
            MainWindow::instance()->setRightPages(MainWindow::ChatPage);
            // TODO: 设置当前聊天对象
            // ChatService::getInstance().setActiveChatUserId(friendId);
        });
        
        m_containerLayout->insertWidget(i, item);
    }
}

void ContactList::searchContacts(const QString& keyword)
{
    if (keyword.isEmpty()) {
        // 如果搜索关键词为空，显示所有联系人
        displayContacts(m_allContacts);
        return;
    }
    
    // 模糊搜索：匹配昵称和备注
    QJsonArray filteredContacts;
    QString lowerKeyword = keyword.toLower();  // 转换为小写进行不区分大小写搜索
    
    for (int i = 0; i < m_allContacts.size(); ++i) {
        QJsonObject contactObj = m_allContacts[i].toObject();
        QString displayName = contactObj["displayName"].toString().toLower();
        QString nickname = contactObj["nickname"].toString().toLower();
        QString remark = contactObj["remark"].toString().toLower();
        
        // 检查是否匹配昵称、备注或显示名称
        if (displayName.contains(lowerKeyword) || 
            nickname.contains(lowerKeyword) || 
            remark.contains(lowerKeyword)) {
            filteredContacts.append(contactObj);
        }
    }
    
    // 显示搜索结果
    displayContacts(filteredContacts);
}

void ContactList::clearContacts()
{
    // 清空所有联系人项
    QLayoutItem* item;
    while ((item = m_containerLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    m_itemsById.clear();
    m_containerLayout->addStretch();
}

ContactList::~ContactList()
{}



