#include "lanchat_demo01.h"
#include "./ui_lanchat_demo01.h"
#include "sidebar.h"
#include "chatpage.h"

LanChat_demo01::LanChat_demo01(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LanChat_demo01)
{
    ui->setupUi(this);

    this->setMinimumSize(760, 600);

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 无边距
    mainLayout->setSpacing(0);

    SideBar *sidebar = new SideBar(this);
    mainLayout->addWidget(sidebar);

    ChatPage* chatpage = new ChatPage(this);
    mainLayout->addWidget(chatpage);
}

LanChat_demo01::~LanChat_demo01()
{
    delete ui;
}
