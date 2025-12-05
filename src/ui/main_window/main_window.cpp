// main_window.cpp
/*
 * 页面名称：登录后主窗口
 * 功能：显示消息列表
 * 用法：
 */

// src/ui/main_window/main_window.cpp
#include "main_window.h"
#include "ChatWindow.h"
#include "ContactList.h"
#include "MessageList.h"
#include "ui/setting/settingdialog.h"
#include "ui/personinfo/ProfileEditDialog.h"
#include "ui/personinfo/UserProfile.h"
#include "ui/personinfo/UserEntity.h"
#include "ui/personinfo/ProfileViewDialog.h"
#include "add_friend_dialog.h"
#include "friend_request_item.h"
#include "service/auth_service.h"
#include "utils/logger.h"
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
    QWidget* recentPage = new MessageList(this);
    m_contactPages->addWidget(recentPage);

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
    m_middleWidget->setFixedWidth(200);
}

//右侧


void MainWindow::setupPages()
{
    //===================== 创建页面 =====================
    m_pages = new QStackedWidget(this);

    //聊天框
    Logger::getInstance().log("[MainWindow] Creating ChatWindow...");
    m_chatPage = new ChatWindow(this);
    Logger::getInstance().log(QString("[MainWindow] ChatWindow created at: %1").arg((quintptr)m_chatPage, 0, 16));
    
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