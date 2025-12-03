#include "chatpage.h"
#include "ui_chatpage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include "sessionitemwidget.h"

ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(1);   // 分割线

    // 左侧容器
    m_leftWidget = new QWidget(this);
    m_leftWidget->setMinimumWidth(200);
    m_leftWidget->setStyleSheet("background-color: #f5f5f5;");

    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftWidget);
    leftLayout->setContentsMargins(10, 10, 10, 0); // 设置边距
    leftLayout->setSpacing(10); // 搜索框和列表之间的间距

    // 搜索框
    m_searchEdit = new QLineEdit(m_leftWidget);
    m_searchEdit->setPlaceholderText("搜索");
    m_searchEdit->setFixedHeight(30);
    // 搜索框样式
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #e2e2e2;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding-left: 5px;" // 文字左边距
        "   color: #333333;"
        "}"
        "QLineEdit:focus {"
        "   background-color: #ffffff;" // 聚焦变白
        "}"
        );
    // 搜索图标
    QAction *searchAction = m_searchEdit->addAction(QIcon(":/img/search.svg"), QLineEdit::LeadingPosition);

    // Sessionlist设置
    m_sessionList = new QListWidget(this);
    m_sessionList->setFrameShape(QFrame::NoFrame); // 去除边框
    // Sessionlist样式
    m_sessionList->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "   outline: 0;"
        "}"
        "QListWidget::item {"
        "   background-color: transparent;"
        "   border-radius: 8px;"
        "   margin: 2px 0px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #e6e6e6;"
        "   color: black;"  // 显式设置选中文字颜色
        "}"
        "QListWidget::item:hover {"
        "   background-color: #f0f0f0;"
        "}"
        "QListWidget QWidget {"
        "   background-color: transparent;"
        "}"
        "QListWidget QLabel {"
        "   background-color: transparent;"
        "}"
        // 强制保持和 selected 一样的颜色
        "QListWidget::item:selected:hover {"
        "   background-color: #e6e6e6;"
        "}"
        );
    // 将组件加入左侧垂直布局
    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(m_sessionList);

    // ==========================================
    // 右侧聊天区域
    m_rightStackedWidget = new QStackedWidget(this);
    // 1. 创建空白页 (Page 0)
    m_emptyWidget = new QWidget(this);
    m_emptyWidget->setStyleSheet("background-color: #f5f5f5;");
    m_emptyWidget->setMinimumWidth(360);
    // QVBoxLayout *emptyLayout = new QVBoxLayout(m_emptyWidget);
    // QLabel *lblIcon = new QLabel(m_emptyWidget);
    // lblIcon->setPixmap(QPixmap(":/img/wechat_logo_gray.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 找个灰色的图标
    // lblIcon->setAlignment(Qt::AlignCenter);
    // QLabel *lblText = new QLabel("暂无会话选中", m_emptyWidget);
    // lblText->setStyleSheet("color: #999999; font-size: 14px;");
    // lblText->setAlignment(Qt::AlignCenter);

    m_chatWindow = new ChatWindow(this);

    m_rightStackedWidget->addWidget(m_emptyWidget); // Index 0
    m_rightStackedWidget->addWidget(m_chatWindow);  // Index 1
    m_rightStackedWidget->setCurrentIndex(0);

    // 设置右侧最小宽度 360px
    m_chatWindow->setMinimumWidth(360);

    // 将组件加入 Splitter
    m_splitter->addWidget(m_leftWidget);
    m_splitter->addWidget(m_rightStackedWidget);

    // 设置伸缩因子：index 1 (右侧) 只有在窗口拉大时会变大，index 0 (左侧) 保持相对固定
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    // 禁止将左侧列表拖拽到消失
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);

    mainLayout->addWidget(m_splitter);

    // 信号连接
    connect(m_sessionList, &QListWidget::itemClicked, this, &ChatPage::onSessionClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ChatPage::onSearchTextChanged);
    initMockData(); // 模拟填充左侧列表
}

void ChatPage::initMockData()
{
    // 模拟数据源
    struct MockInfo {
        QString id;
        QString name;
        QString msg;
        QString time;
    };

    QList<MockInfo> dataList = {
        {"user_1", "SCSE-2025硕士", "助管-张贤玉: @全体成员 请各位同学注意...", "星期一"},
        {"user_2", "7063-31211", "ok", "星期日"},
        {"user_3", "文件传输助手", "[图片] IMG_2025.jpg", "10:30"},
        {"user_4", "产品部交流群", "李四: 下午两点开会", "昨天"}
    };

    // 填充列表
    for (const auto &info : dataList) {
        // 创建 QListWidgetItem
        QListWidgetItem *item = new QListWidgetItem(m_sessionList);

        // 设置 Item 的大小
        item->setSizeHint(QSize(200, 70)); // 宽度随 ListWidget，高度固定 70px

        // 存ID和名字（为了后续搜索使用）
        item->setData(RoleUserId, info.id);
        item->setData(RoleUserName, info.name);

        // 创建自定义 Widget
        SessionItemWidget *customWidget = new SessionItemWidget(m_sessionList);
        customWidget->setTitle(info.name);
        customWidget->setMessage(info.msg);
        customWidget->setTime(info.time);
        customWidget->setAvatar(":/img/member1.jpg"); // 头像路径

        // 将 Widget 放入 Item 中
        m_sessionList->setItemWidget(item, customWidget);
    }
}

void ChatPage::onSessionClicked(QListWidgetItem *item)
{
    if (!item) return;

    // 取出ID和名字
    QString userId = item->data(RoleUserId).toString();
    QString userName = item->data(RoleUserName).toString();

    m_chatWindow->switchChat(userId, userName);
    // 切换到聊天界面
    if (m_rightStackedWidget->currentIndex() != 1) {
        m_rightStackedWidget->setCurrentIndex(1);
    }
}

void ChatPage::onSearchTextChanged(const QString &text)
{
    // 遍历列表中的所有项
    for (int i = 0; i < m_sessionList->count(); ++i) {
        QListWidgetItem *item = m_sessionList->item(i);

        // 获取之前存储的用户名称 (RoleUserName)
        QString name = item->data(RoleUserName).toString();

        // 如果搜索框为空，显示所有；否则匹配名称
        bool isMatch = name.contains(text, Qt::CaseInsensitive); // 不区分大小写
        item->setHidden(!isMatch);
    }
}

ChatPage::~ChatPage()
{
    delete ui;
    m_chatWindows.clear();
}
