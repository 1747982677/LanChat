#include "sidebar.h"
#include "ui_sidebar.h"
#include "pixmaputils.h"

SideBar::SideBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SideBar)
{
    ui->setupUi(this);

    // 固定宽度60px
    this->setFixedWidth(60);
    // 背景色
    this->setStyleSheet("background-color: #F5F5F5;");

    // 布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 20, 0, 20); // 上下留白
    layout->setSpacing(10);

    // 头像
    QString path = ":/img/user.jpg";
    QPixmap avatar = PixmapUtils::clipToCircle(path, 36);
    m_avatar = new QLabel(this);
    m_avatar->setPixmap(avatar);
    m_avatar->setFixedSize(36, 36);
    m_avatar->setScaledContents(true);
    m_avatar->setStyleSheet("background-color: transparent;");


    // 居中头像
    layout->addWidget(m_avatar, 0, Qt::AlignHCenter);

    QVBoxLayout *sideLayout = new QVBoxLayout;
    sideLayout->setAlignment(Qt::AlignHCenter);
    sideLayout->setSpacing(10);
    sideLayout->setContentsMargins(0, 10, 0, 20);

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);

    // 聊天按钮
    m_btnChat = new SideBarButton("聊天",
                                  ":/img/chat_gray.svg",
                                  ":/img/chat_highlight.svg");
    sideLayout->addWidget(m_btnChat);
    m_btnGroup->addButton(m_btnChat, 0); // ID 0

    // 通讯录按钮
    m_btnContact = new SideBarButton("通讯录",
                                     ":/img/contact_gray.svg",
                                     ":/img/contact_highlight.svg");
    sideLayout->addWidget(m_btnContact);
    m_btnGroup->addButton(m_btnContact, 1); // ID 1

    // 设置按钮
    m_btnSetting = new SideBarButton("设置",
                                     ":/img/setting_gray.svg",
                                     ":/img/setting_highlight.svg");
    sideLayout->addWidget(m_btnSetting);
    m_btnGroup->addButton(m_btnSetting, 2); // ID 2

    layout->addLayout(sideLayout);

    layout->addStretch();

    // 设置默认选中
    m_btnChat->setChecked(true);

}

SideBar::~SideBar()
{
    delete ui;
}
