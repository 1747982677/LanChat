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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include<QPixmap>

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
    tabLayout->addWidget(m_avatarLabel, 0, Qt::AlignCenter);

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

//中间
void MainWindow::setupMiddleColumn()
{
    // 中间页
    m_middleWidget = new QWidget(this);
    auto* middleLayout = new QVBoxLayout(m_middleWidget);
    middleLayout->setContentsMargins(0,20,8,0);
    middleLayout->setSpacing(0);

    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索联系人/会话");
    m_searchEdit->setClearButtonEnabled(true);
    middleLayout->addWidget(m_searchEdit);

    
    m_contactPages = new QStackedWidget(this);
    // ===================== 消息列表 =====================
    QWidget* recentPage = new MessageList(this);
    m_contactPages->addWidget(recentPage);

    // ===================== 联系人列表  =====================
    QWidget* friendsPage = new ContactList(this);
    m_contactPages->addWidget(friendsPage);


    middleLayout->addWidget(m_contactPages);
    m_middleWidget->setFixedWidth(200);
}

//右侧


void MainWindow::setupPages()
{
    //===================== 创建页面 =====================
    m_pages = new QStackedWidget(this);

    //聊天框
    m_chatPage = new ChatWindow(this);
    auto* chatLayout = new QVBoxLayout(m_chatPage);
    chatLayout->addWidget(new QLabel("聊天框页"));
    chatLayout->addStretch();
    
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
    nfLayout->addWidget(new QLabel("新朋友页"));
    nfLayout->addStretch();

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
    }
}

MainWindow::~MainWindow()
{
    m_instance = nullptr;
}