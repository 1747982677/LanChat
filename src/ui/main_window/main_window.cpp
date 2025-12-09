// main_window.cpp
/*
 * 页面名称：登录后主窗口
 * 功能：显示消息列表
 * 用法：
 */

// src/ui/main_window/main_window.cpp
#include "main_window.h"
#include "ChatWindow.h"
#include "SessionInfo.h"
#include "ContactList.h"
#include "MessageList.h"
#include "ui/setting/settingdialog.h"
#include "ui/personinfo/ProfileEditDialog.h"
#include "ui/personinfo/UserProfile.h"
#include "ui/personinfo/UserEntity.h"
#include "ui/personinfo/ProfileViewDialog.h"
#include "add_friend_dialog.h"
#include "friend_request_item.h"
#include "SessionListItem.h"
#include "service/auth_service.h"
#include "utils/logger.h"
#include "utils/db_manager.h"
#include "core/network_controller.h"
#include "core/app_context.h"
#include "common/types.h"
#include "model/message_dao.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include<QPixmap>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "core/app_context.h"
#include "core/dblogic_controller.h"

MainWindow* MainWindow::m_instance = nullptr;

MainWindow* MainWindow::instance()
{
    if (!m_instance) {
        m_instance = new MainWindow();
    }
    return m_instance;
}

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    connect(dbCtrl, &DbLogicController::updateUserReady,
        this, &MainWindow::updateUserReady);
    bool connected = connect(dbCtrl, &DbLogicController::queryUserReady,
        this, &MainWindow::queryUserReady);
    Logger::getInstance().log(QString("[MainWindow] Signal connection result: %1").arg(connected ? "SUCCESS" : "FAILED"));

    if (connected) {
        Logger::getInstance().log("[MainWindow] Signal connected to DbLogicController");
    }
    else {
        Logger::getInstance().error("[MainWindow] FAILED to connect signal!");
    }
    
    // 连接好友申请相关信号
    connect(dbCtrl, &DbLogicController::friendRequestsLoaded,
            this, &MainWindow::onFriendRequestsLoaded);
    connect(dbCtrl, &DbLogicController::friendRequestAccepted,
            this, &MainWindow::onFriendRequestAccepted);

    connect(m_sessionList, &SessionList::sessionSelected,
        m_chatPage, &ChatWindow::setSessionInfo);
    
    // 当会话被选中时，更新未读消息提示（因为会清除该会话的未读数）
    connect(m_sessionList, &SessionList::sessionSelected,
        this, [this](const SessionInfo&) {
            updateUnreadBadge();
        });
}
void MainWindow::updateUserReady(const bool& glag)
{
    if (glag)
    {
        QMessageBox::warning(this, "警告", "更新用户信息成功！！");
        Logger::getInstance().log("[MainWindow] 更新用户信息成功！");
        /*requestQueryUser();*/
    }
    else
    {
        Logger::getInstance().log("[MainWindow] 更新用户信息失败！");
	}
}

void MainWindow::requestQueryUser()
{
    UserEntity user(userid, "", "", "");
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    dbCtrl->requestQueryUser(user);
}

void MainWindow::queryUserReady(const UserEntity& localUser)
{
    m_currentUser = localUser;
    Logger::getInstance().log("[MainWindow] 加载当前用户信息成功！");
    
    // 设置用户状态为在线（登录成功后）
    updateUserStatus(true);
    
    // 加载联系人列表
    // 找到 ContactList 并加载联系人
    QWidget* contactPage = m_contactPages->widget(1);  // 联系人页面是第二个（索引1）
    if (contactPage) {
        ContactList* contactList = qobject_cast<ContactList*>(contactPage);
        if (contactList) {
            contactList->loadContacts();
        }
    }
}

void MainWindow::updateUserStatus(bool isOnline)
{
    if (!m_statusLabel) {
        return;
    }
    
    if (isOnline) {
        m_statusLabel->setText("在线");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   color: #07c160;"
            "   font-size: 12px;"
            "   padding: 2px 8px;"
            "   background-color: transparent;"
            "}"
        );
    } else {
        m_statusLabel->setText("离线");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   color: #999999;"
            "   font-size: 12px;"
            "   padding: 2px 8px;"
            "   background-color: transparent;"
            "}"
        );
    }
}
//左侧栏
void MainWindow::setupLeftNav()
{
    m_tabBar = new QWidget(this);
    auto* tabLayout = new QVBoxLayout(m_tabBar);
    tabLayout->setContentsMargins(20,20,10,20);
    tabLayout->setSpacing(12);

    // ===================== 头像区域 =====================
    m_avatarLabel = new QLabel(m_tabBar);
    m_avatarLabel->setFixedSize(60,60);
    m_avatarLabel->setScaledContents(true);
    QPixmap pixmap(":/lanchat/bubu.jpg"); 
    if (!pixmap.isNull()) {
        m_avatarLabel->setPixmap(pixmap);
        m_avatarLabel->setStyleSheet("QLabel { border-radius: 50%; }");
    }
    else {
        m_avatarLabel->setText("头像\n加载失败");
        m_avatarLabel->setAlignment(Qt::AlignCenter);
        m_avatarLabel->setStyleSheet(
            "QLabel {"
            "border-radius: 50%;"
            "background-color: #666666;"
            "}"
        );
    }
    // 2. 安装事件过滤器
    m_avatarLabel->installEventFilter(this);
    tabLayout->addWidget(m_avatarLabel, 0, Qt::AlignCenter);
    
    // ===================== 状态标签 =====================
    m_statusLabel = new QLabel("离线", m_tabBar);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #999999;"
        "   font-size: 12px;"
        "   padding: 2px 8px;"
        "   background-color: transparent;"
        "}"
    );
    tabLayout->addWidget(m_statusLabel, 0, Qt::AlignCenter);

    // ===================== 侧边栏按钮区域 =====================

    m_sideButtonGroup = new QButtonGroup(this);
    m_sideButtonGroup->setExclusive(true);

    // 按钮文字列表（想加按钮只改这里）
    QStringList buttonTexts = { "消息", "联系人", "设置","可扩展"};

    for (int i = 0; i < buttonTexts.size(); ++i)
    {
        QPushButton* btn = new QPushButton(buttonTexts[i], m_tabBar);
        btn->setCheckable(true);
        btn->setMinimumHeight(36);

        btn->setStyleSheet(
            "QPushButton {border:none;border-radius:6px;}"
            "QPushButton:checked {background:#555; color:white;}"
            "QPushButton:hover {background:#333; color:white;}"
        );

        tabLayout->addWidget(btn);
        m_sideButtonGroup->addButton(btn, i);
        
        // 保存"消息"按钮的引用（第一个按钮）
        if (i == 0) {
            m_messageButton = btn;
            
            // 创建未读消息红点标签
            m_unreadBadge = new QLabel(btn);
            m_unreadBadge->setAlignment(Qt::AlignCenter);
            m_unreadBadge->setStyleSheet(
                "QLabel {"
                "   background-color: #ff4d4f;"
                "   color: white;"
                "   border-radius: 9px;"
                "   font-size: 11px;"
                "   font-weight: bold;"
                "   padding: 1px 5px;"
                "}"
            );
            m_unreadBadge->setMinimumSize(18, 18);
            m_unreadBadge->hide();  // 初始隐藏
            
            // 定位红点到按钮右上角
            m_unreadBadge->move(btn->width() - 20, 5);
        }
    }

    tabLayout->addStretch();
    m_tabBar->setFixedWidth(100);

    // 切换右侧主页面 
    // 中间页面|右侧页面 0:消息列表|聊天框   1:联系人|联系人消息  2:设置弹窗
    connect(m_sideButtonGroup, &QButtonGroup::idClicked, this,
        [this](int id) {

            PageType type = static_cast<PageType>(id);
            if (type == PageType::ChatPage || type == PageType::FriendInfo) {
                m_contactPages->setCurrentIndex(id);
                setRightPages(type);   // 使用你封装好的页面切换函数
                
                // 如果切换到消息页面，清除未读消息提示
                if (type == PageType::ChatPage) {
                    updateUnreadBadge();
                }
            }
            // 2: 设置 → 弹出设置弹窗
            else if (type == SettingsPage) {
                openSettingsDialog();   
                return;
            }
            else {

            }
            
        });

    // 默认选中第一个
    m_sideButtonGroup->button(0)->setChecked(true);

}

// 实现 eventFilter
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_avatarLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                qDebug() << "头像被点击了！";
				showProfileViewDialog(); // 显示个人信息查看对话框
                return true;
            }
        }
    }
    return false;
}
void MainWindow::showProfileViewDialog()
{
    if (!m_currentUser.isValid()) {
        Logger::getInstance().log("[MainWindow] 用户信息不完整，无法显示个人资料");
        return;
    }
    /*userid*/
    m_userProfile = UserProfile();
	m_userProfile.userid = m_currentUser.userId;
    m_userProfile.nickname = m_currentUser.nickname;
    m_userProfile.email = m_currentUser.email;
    m_userProfile.phone = m_currentUser.phone;
    m_userProfile.signure = m_currentUser.signature;
	m_userProfile.password = m_currentUser.passwordHash;  // 注意：UserProfile.password 存储的是密码哈希
    m_userProfile.rootpath = "C:/mty/QtProject/LanChat/src";//请换成你的绝对路径
	m_userProfile.avatarpath = m_userProfile.rootpath +m_currentUser.avatarPath;
    QPixmap pixmap(m_userProfile.avatarpath);
    m_userProfile.avatar = pixmap;
    // 创建并显示个人信息查看对话框
    ProfileViewDialog* viewDialog = new ProfileViewDialog(m_userProfile, this);
    viewDialog->setModal(true);

    // 连接编辑请求信号
    connect(viewDialog, &ProfileViewDialog::editRequested, this, [this, viewDialog]() {
        qDebug() << "用户请求编辑资料";
        viewDialog->close();  // 关闭查看对话框

        // 显示编辑对话框
        ProfileEditDialog* editDialog = new ProfileEditDialog( m_userProfile, this);

        if (editDialog->exec() == QDialog::Accepted) {
            // 更新用户资料
            m_userProfile = editDialog->getUpdatedProfile();

            // 更新主窗口头像
            m_avatarLabel->setPixmap(m_userProfile.avatar);
            m_currentUser.avatarPath = m_userProfile.avatarpath;
			m_currentUser.nickname = m_userProfile.nickname;
			m_currentUser.email = m_userProfile.email;
            m_currentUser.phone = m_userProfile.phone;
            
            /*UserEntity user(userid, "", "", "");*/
            DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
            dbCtrl->requesUpdateUser(m_currentUser);

            qDebug() << "资料已更新";

            // 重新显示查看对话框
            showProfileViewDialog();
        }

        delete editDialog;
        });

    viewDialog->exec();
    delete viewDialog;
}
//中间
void MainWindow::setupMiddleColumn()
{
    // 中间页
    m_middleWidget = new QWidget(this);
    auto* middleLayout = new QVBoxLayout(m_middleWidget);
    middleLayout->setContentsMargins(0,20,8,0);
    middleLayout->setSpacing(0);

    // 搜索框和加号按钮容器
    QWidget* searchContainer = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    
    m_searchEdit = new QLineEdit(searchContainer);
    m_searchEdit->setPlaceholderText("搜索联系人");
    m_searchEdit->setClearButtonEnabled(true);
    
    // 加号按钮（添加朋友）
    QPushButton* addFriendButton = new QPushButton("+", searchContainer);
    addFriendButton->setFixedSize(30, 30);
    addFriendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #07c160;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #06ad56;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #059648;"
        "}"
    );
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(addFriendButton);
    
    middleLayout->addWidget(searchContainer);
    
    // 连接加号按钮点击事件
    connect(addFriendButton, &QPushButton::clicked, this, [this]() {
        AddFriendDialog* dialog = new AddFriendDialog(this);
        dialog->exec();
    });

    
    m_contactPages = new QStackedWidget(this);
    // ===================== 消息列表 =====================
    m_sessionList = new SessionList(this);
    m_contactPages->addWidget(m_sessionList); // index 0

    // ===================== 联系人列表  =====================
    QWidget* friendsPage = new ContactList(this);
    m_contactPages->addWidget(friendsPage);
    
    // 连接搜索框到联系人列表的搜索功能
    ContactList* contactList = qobject_cast<ContactList*>(friendsPage);
    if (contactList) {
        connect(m_searchEdit, &QLineEdit::textChanged, this, [contactList](const QString& text) {
            // 只在联系人页面时进行搜索
            MainWindow* mainWindow = MainWindow::instance();
            if (mainWindow && mainWindow->m_contactPages->currentIndex() == 1) {
                contactList->searchContacts(text);
            }
        });
    }

    middleLayout->addWidget(m_contactPages);
    m_middleWidget->setFixedWidth(310);
}

//右侧


void MainWindow::setupPages()
{
    //===================== 创建页面 =====================
    m_pages = new QStackedWidget(this);

    //聊天框
    Logger::getInstance().log("[MainWindow] Creating ChatWindow (ui/chatpage version)...");
    m_chatPage = new ChatWindow(this);
    Logger::getInstance().log(QString("[MainWindow] ChatWindow (chatpage) created at: %1").arg((quintptr)m_chatPage, 0, 16));
    
    // 连接发送消息信号
    connect(m_chatPage, &ChatWindow::sigSendMessage,
            this, &MainWindow::onSendMessage);
    
    // 连接接收消息信号 - 更新消息列表
    NetworkController* netCtrl = AppContext::instance().networkController();
    if (netCtrl) {
        connect(netCtrl, &NetworkController::messageReceived,
                this, &MainWindow::onMessageReceived);
        Logger::getInstance().log("[MainWindow] Connected to NetworkController::messageReceived for SessionList updates");
    } else {
        Logger::getInstance().error("[MainWindow] NetworkController is null, cannot connect messageReceived signal!");
    }
    
    // 联系人资料
    m_friendInfoPage = new QWidget(this);
    auto* fIndoLayout = new QVBoxLayout(m_friendInfoPage);
    fIndoLayout->addWidget(new QLabel("联系人资料页"));
    fIndoLayout->addStretch();

    //设置页
   /* m_settingsPage = new QWidget(this);
    auto* settingLayout = new QVBoxLayout(m_settingsPage);
    settingLayout->addWidget(new QLabel("设置页"));
    settingLayout->addStretch();*/
   
    //新朋友
    m_newFriendPage = new QWidget(this);
    auto* nfLayout = new QVBoxLayout(m_newFriendPage);
    nfLayout->setContentsMargins(0, 0, 0, 0);
    nfLayout->setSpacing(0);
    
    // 标题
    QLabel* titleLabel = new QLabel("好友申请", m_newFriendPage);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; padding: 15px; background-color: #f5f5f5;");
    nfLayout->addWidget(titleLabel);
    
    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea(m_newFriendPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: white; }");
    
    QWidget* scrollContent = new QWidget();
    m_friendRequestLayout = new QVBoxLayout(scrollContent);
    m_friendRequestLayout->setContentsMargins(0, 0, 0, 0);
    m_friendRequestLayout->setSpacing(0);
    m_friendRequestLayout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    nfLayout->addWidget(scrollArea);

    //===================== 注册页面 =====================
    addRightPage(ChatPage, m_chatPage);
    addRightPage(FriendInfo, m_friendInfoPage);
    //addRightPage(SettingsPage, m_settingsPage);
    addRightPage(NewFriendPage, m_newFriendPage);

}
void MainWindow::openSettingsDialog()
{
    // 以当前 MainWindow 作为父窗口
    SettingDialog dlg(this);
    dlg.exec();   // 模态对话框（阻塞当前，直到关闭）
}

void MainWindow::openChatPage(const QString& userId, const QString& displayName)
{
    // 切换右侧到聊天页
    setRightPages(ChatPage);

    // 将右侧页面转换为 ChatWindow，并切换当前聊天对象
    ChatWindow* chatWin = qobject_cast<ChatWindow*>(m_chatPage);
    if (chatWin) {
        SessionInfo info(userId, displayName);
        // 将会话写入会话列表，确保 uid 使用真实的对方 userId
        if (m_sessionList) {
            info.setAvatarPath(""); // 头像可按需补充
            m_sessionList->upsertSession(info);
        }
        chatWin->setSessionInfo(info);
    }
}

void MainWindow::onSendMessage(const QString& targetUid, const UiMessage& msg)
{
    qDebug() << "[MainWindow] onSendMessage called - target:" << targetUid << "content:" << msg.content;
    Logger::getInstance().log(QString("[MainWindow] onSendMessage called - target: %1, content: %2")
                             .arg(targetUid).arg(msg.content));
    
    // 构造 LanChat::Message 对象
    LanChat::Message lanChatMsg;
    lanChatMsg.messageId = msg.mid.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : msg.mid;
    // 使用当前登录用户ID
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    lanChatMsg.senderId = currentUserId;
    lanChatMsg.receiverId = targetUid;
    lanChatMsg.content = msg.content;
    lanChatMsg.timestamp = msg.timestamp.toMSecsSinceEpoch();  // 用毫秒时间戳
    lanChatMsg.type = LanChat::MessageType::Text;
    lanChatMsg.status = LanChat::MessageStatus::Pending;
    
    // 先写入本地数据库，保持历史可见
    {
        Message daoMsg;
        daoMsg.sender = lanChatMsg.senderId;
        daoMsg.receiver = lanChatMsg.receiverId;
        daoMsg.content = lanChatMsg.content;
        daoMsg.timestamp = QDateTime::fromMSecsSinceEpoch(lanChatMsg.timestamp);
        daoMsg.status = static_cast<int>(lanChatMsg.status);
        MessageDao::insertMessage(daoMsg);
    }

    qDebug() << "[MainWindow] Calling NetworkController::sendMessage";
    // 通过 NetworkController 发送消息
    NetworkController* netCtrl = AppContext::instance().networkController();
    if (netCtrl) {
        netCtrl->sendMessage(lanChatMsg);
        Logger::getInstance().log("[MainWindow] Message sent to NetworkController");
        qDebug() << "[MainWindow] Message sent to NetworkController";
    } else {
        Logger::getInstance().error("[MainWindow] NetworkController is null!");
        qDebug() << "[MainWindow] NetworkController is null!";
    }
    qDebug() << "[MainWindow] onSendMessage finished";
}
//根据发送方的Id获取账号、备注等信息
QString MainWindow::getUserDisplayName(const QString& userId)
{
    Logger::getInstance().log(QString("[MainWindow] getUserDisplayName called for userId: %1").arg(userId));
    
    // 获取当前登录用户ID
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    
    if (currentUserId.isEmpty()) {
        Logger::getInstance().warning("[MainWindow] Cannot get user display name: not logged in");
        return userId;
    }
    
    Logger::getInstance().log(QString("[MainWindow] Current logged in user: %1").arg(currentUserId));
    
    // 查询数据库获取用户信息
    QSqlDatabase db = DatabaseManager::getInstance().database();
    if (!db.isOpen()) {
        Logger::getInstance().error("[MainWindow] Database is not open");
        return userId;
    }
    
    QString displayName = userId;  // 默认使用userId
    
    // 1. 先查询是否有备注（从friends表）
    QSqlQuery friendQuery(db);
    friendQuery.prepare("SELECT remark FROM friends WHERE userId = :currentUserId AND friendId = :friendId");
    friendQuery.bindValue(":currentUserId", currentUserId);
    friendQuery.bindValue(":friendId", userId);
    
    Logger::getInstance().log(QString("[MainWindow] Querying friends table for remark..."));
    QString remark;
    if (friendQuery.exec() && friendQuery.next()) {
        remark = friendQuery.value("remark").toString();
        Logger::getInstance().log(QString("[MainWindow] Friend query result - remark: '%1'").arg(remark));
        if (!remark.isEmpty()) {
            displayName = remark;
            Logger::getInstance().log(QString("[MainWindow] Using remark as displayName: %1").arg(remark));
            return displayName;
        }
    } else {
        Logger::getInstance().log(QString("[MainWindow] No friend record found or query failed: %1")
                                 .arg(friendQuery.lastError().text()));
    }
    
    // 2. 查询用户的昵称和邮箱（从public.db的users表）
    QSqlDatabase publicDb = DatabaseManager::getInstance().database("public");
    if (!publicDb.isOpen()) {
        Logger::getInstance().error("[MainWindow] Public database is not open");
        return userId;
    }
    
    QSqlQuery userQuery(publicDb);
    userQuery.prepare("SELECT nickname, email FROM users WHERE userId = :userId");
    userQuery.bindValue(":userId", userId);
    
    Logger::getInstance().log(QString("[MainWindow] Querying public.db users table..."));
    if (userQuery.exec() && userQuery.next()) {
        QString nickname = userQuery.value("nickname").toString();
        QString email = userQuery.value("email").toString();
        
        Logger::getInstance().log(QString("[MainWindow] User query result - nickname: '%1', email: '%2'")
                                 .arg(nickname).arg(email));
        
        // 优先使用昵称
        if (!nickname.isEmpty()) {
            displayName = nickname;
            Logger::getInstance().log(QString("[MainWindow] Using nickname as displayName: %1").arg(nickname));
        }
        // 其次使用邮箱
        else if (!email.isEmpty()) {
            displayName = email;
            Logger::getInstance().log(QString("[MainWindow] Using email as displayName: %1").arg(email));
        }
    } else {
        Logger::getInstance().warning(QString("[MainWindow] User not found in public database: %1, error: %2")
                                     .arg(userId).arg(userQuery.lastError().text()));
    }
    
    Logger::getInstance().log(QString("[MainWindow] Final displayName for %1: %2").arg(userId).arg(displayName));
    return displayName;
}

void MainWindow::onMessageReceived(const QJsonObject& msgJson, const QString& from)
{
    qDebug() << "[MainWindow] onMessageReceived called - from:" << from;
    Logger::getInstance().log(QString("[MainWindow] Message received from: %1").arg(from));
    
    // 反序列化消息
    LanChat::Message msg = LanChat::Message::fromJson(msgJson);
    
    // 构造 UiMessage
    UiMessage uiMsg;
    uiMsg.mid = msg.messageId;
    uiMsg.senderId = msg.senderId;
    uiMsg.content = msg.content;
    uiMsg.timestamp = QDateTime::fromMSecsSinceEpoch(msg.timestamp);
    uiMsg.isSelf = false;
    
    // 查找或创建会话信息
    SessionInfo sessionInfo;
    bool sessionFound = false;
    
    // 遍历现有会话列表，查找是否已存在该会话
    for (int i = 0; i < m_sessionList->count(); ++i) {
        QWidget* w = m_sessionList->itemWidget(m_sessionList->item(i));
        SessionListItem* sItem = qobject_cast<SessionListItem*>(w);
        if (sItem && sItem->getData().uid() == msg.senderId) {
            sessionInfo = sItem->getData();
            sessionFound = true;
            break;
        }
    }
    
    // 如果会话不存在，创建新会话并查询发送者信息
    if (!sessionFound) {
        sessionInfo.setUid(msg.senderId);
        
        // 从数据库查询发送者的显示名称
        QString displayName = getUserDisplayName(msg.senderId);
        sessionInfo.setUsername(displayName);
        sessionInfo.setAvatarPath(""); // 可以后续从数据库加载头像
        Logger::getInstance().log(QString("[MainWindow] Creating new session for: %1 with displayName: %2")
                                 .arg(msg.senderId).arg(displayName));
    }
    
    // 更新会话信息
    sessionInfo.addNewMessage(uiMsg);
    
    // 更新或插入会话到列表
    m_sessionList->upsertSession(sessionInfo);
    
    // 重新排序会话列表（将有新消息的会话置顶）
    m_sessionList->sortSessions();
    
    // 更新侧边栏未读消息提示
    updateUnreadBadge();
    
    Logger::getInstance().log(QString("[MainWindow] SessionList updated for: %1, unread count: %2")
                             .arg(msg.senderId).arg(sessionInfo.unreadCount()));
    qDebug() << "[MainWindow] SessionList updated successfully";
}
//获取
int MainWindow::getTotalUnreadCount()
{
    int totalUnread = 0;
    
    // 遍历所有会话，累加未读消息数
    for (int i = 0; i < m_sessionList->count(); ++i) {
        QWidget* w = m_sessionList->itemWidget(m_sessionList->item(i));
        SessionListItem* sItem = qobject_cast<SessionListItem*>(w);
        if (sItem) {
            totalUnread += sItem->getData().unreadCount();
        }
    }
    
    return totalUnread;
}

void MainWindow::updateUnreadBadge()
{
    if (!m_unreadBadge || !m_messageButton) {
        return;
    }
    
    int totalUnread = getTotalUnreadCount();
    
    if (totalUnread > 0) {
        // 显示未读消息数
        if (totalUnread > 99) {
            m_unreadBadge->setText("99+");
        } else {
            m_unreadBadge->setText(QString::number(totalUnread));
        }
        m_unreadBadge->adjustSize();
        m_unreadBadge->setMinimumSize(18, 18);
        
        // 重新定位红点到按钮右上角
        int badgeX = m_messageButton->width() - m_unreadBadge->width() - 5;
        int badgeY = 5;
        m_unreadBadge->move(badgeX, badgeY);
        
        m_unreadBadge->show();
        Logger::getInstance().log(QString("[MainWindow] Unread badge updated: %1 messages").arg(totalUnread));
    } else {
        // 没有未读消息，隐藏红点
        m_unreadBadge->hide();
        Logger::getInstance().log("[MainWindow] Unread badge hidden (no unread messages)");
    }
}

void MainWindow::setupUi()
{
    resize(900, 500);
    this->setMinimumHeight(500);
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧标签栏
    setupLeftNav();
    mainLayout->addWidget(m_tabBar);

    // 中间页面
    setupMiddleColumn();
    mainLayout->addWidget(m_middleWidget);

    // 右侧页面
    setupPages();
    mainLayout->addWidget(m_pages, 1);
}
void MainWindow::addRightPage(PageType type, QWidget* page)
{
    m_pages->addWidget(page);
    m_pageMap[type] = page;
}

void MainWindow::setRightPages(PageType type)
{
    if (m_pageMap.contains(type)) {
        m_pages->setCurrentWidget(m_pageMap[type]);
        
        // 如果切换到新朋友页面，加载好友申请列表
        if (type == NewFriendPage) {
            loadFriendRequests();
        }
    }
}

void MainWindow::loadFriendRequests()
{
    // 获取当前登录用户ID
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    
    if (currentUserId.isEmpty()) {
        Logger::getInstance().warning("[MainWindow] Cannot load friend requests: user not logged in");
        return;
    }
    
    // 查询好友申请
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    dbCtrl->queryFriendRequests(currentUserId);
}

void MainWindow::onFriendRequestsLoaded(const QJsonArray& requests)
{
    // 清空现有列表
    QLayoutItem* item;
    while ((item = m_friendRequestLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    if (requests.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无好友申请", m_newFriendPage);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #999; font-size: 14px; padding: 50px;");
        m_friendRequestLayout->insertWidget(0, emptyLabel);
        return;
    }
    
    // 添加好友申请项
    for (int i = 0; i < requests.size(); ++i) {
        QJsonObject requestObj = requests[i].toObject();
        QString requestId = requestObj["requestId"].toString();
        QString senderId = requestObj["senderId"].toString();
        QString senderNickname = requestObj["senderNickname"].toString();
        QString senderAccount = requestObj["senderAccount"].toString();
        QString avatarPath = requestObj["avatarPath"].toString();
        QString verifymsg = requestObj["verifymsg"].toString();
        
        FriendRequestItem* item = new FriendRequestItem(
            requestId, senderId, senderNickname, senderAccount, avatarPath, verifymsg, m_newFriendPage);
        
        connect(item, &FriendRequestItem::acceptClicked,
                this, &MainWindow::onAcceptButtonClicked);
        
        m_friendRequestLayout->insertWidget(i, item);
    }
}

void MainWindow::onAcceptButtonClicked(const QString& requestId, const QString& senderId)
{
    // 获取当前登录用户ID
    AuthService& authService = AuthService::getInstance();
    QString receiverId = authService.getCurrentUserId();
    
    if (receiverId.isEmpty()) {
        QMessageBox::warning(this, "错误", "未登录，请先登录");
        return;
    }
    
    // 调用数据库控制器接受好友请求
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    dbCtrl->acceptFriendRequest(requestId, senderId, receiverId);
}

void MainWindow::onFriendRequestAccepted(bool success, const QString& errorMessage)
{
    if (success) {
        QMessageBox::information(this, "成功", "已同意好友申请");
        // 重新加载好友申请列表
        loadFriendRequests();
    } else {
        QMessageBox::warning(this, "失败", errorMessage.isEmpty() ? "同意好友申请失败" : errorMessage);
    }
}

MainWindow::~MainWindow()
{
    m_instance = nullptr;
}