#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QListWidget>
#include <QScrollBar>
#include <QTimer>

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶部标题栏
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(50);
    headerWidget->setStyleSheet("border-bottom: 1px solid #e0e0e0; background: white;");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    m_nameLabel = new QLabel("", headerWidget);
    m_nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addStretch();

    // 中间内容区 (使用 Splitter 分割消息区和输入区)
    QSplitter *vSplitter = new QSplitter(Qt::Vertical, this);
    vSplitter->setHandleWidth(1);
    // 消息列表区
    m_chatView = new NChatView(this);
    connect(m_chatView, &NChatView::sigLoadingHistory, this, &ChatWindow::onLoadMoreHistory);

    // 输入区容器
    QWidget *inputContainer = new QWidget(this);
    inputContainer->setMinimumHeight(150);
    inputContainer->setStyleSheet("background-color: white;");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputContainer);
    inputLayout->setContentsMargins(10, 5, 10, 10);
    // 文本输入框
    m_inputEdit = new QTextEdit(inputContainer);
    m_inputEdit->setFrameShape(QFrame::NoFrame); // 无边框
    m_inputEdit->setPlaceholderText("");
    // 发送按钮区
    QHBoxLayout *sendLayout = new QHBoxLayout();
    sendLayout->addStretch();
    m_sendBtn = new QPushButton("发送", inputContainer);
    m_sendBtn->setFixedSize(80, 30);
    m_sendBtn->setStyleSheet("QPushButton { background-color: #24E0AB; color: white; border-radius: 4px; } QPushButton:hover { background-color: #007acc; }");
    connect(m_sendBtn, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    sendLayout->addWidget(m_sendBtn);

    // 组装输入区
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addLayout(sendLayout);
    // 添加到 Splitter
    vSplitter->addWidget(m_chatView);
    vSplitter->addWidget(inputContainer);
    vSplitter->setStretchFactor(0, 7); // 消息区占大头
    vSplitter->setStretchFactor(1, 3); // 输入区占小头
    // 最终组装主布局
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(vSplitter);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}
// 【核心逻辑】切换用户
void ChatWindow::switchChat(const QString &userId, const QString &userName)
{
    // 如果点的就是当前这人，啥都不做
    if (userId == m_currentUserId) return;

    m_currentUserId = userId;
    m_currentUserName = userName;

    // 更新顶部的名字
    m_nameLabel->setText(userName);

    // 清空旧数据
    m_chatView->clear();
    m_allMessages.clear();
    m_displayedCount = 0;

    // 加载新用户的数据 (模拟从数据库读)
    loadDataFromDatabase(userId);

    // 初始加载第一页 (比如最新的 20 条)
    renderHistoryBatch();
    // 切换新用户时，通常直接滚动到底部看最新消息
    m_chatView->scrollToBottom();
}



// 从数据库读数据
void ChatWindow::loadDataFromDatabase(const QString &userId)
{
    // 模拟生成 100 条历史记录
    for (int i = 0; i < 60; ++i) {
        QString content = QString("History msg %1 from %2").arg(i).arg(userId);
        MsgRole role = (i % 2 == 0) ? MsgRole::Other : MsgRole::Self;
        m_allMessages.append(MessageData(content, role));
    }
}


// 【分页逻辑】加载历史记录
void ChatWindow::renderHistoryBatch()
{
    // 如果已经全部显示完了，就返回
    int total = m_allMessages.count();
    if (m_displayedCount >= total) return;

    // 计算这一批要加载多少条
    // 我们是从 m_allMessages 的末尾往前拿数据的
    // 比如 total=100, displayed=0. 我们要拿 index 85~99 这 15 条
    int start = total - m_displayedCount - 1;
    int end = start - PAGE_SIZE + 1;
    if (end < 0) end = 0; // 防止越界

    // 记录加载前的滚动条高度 (为了保持视觉位置)
    int oldScrollHeight = m_chatView->verticalScrollBar()->maximum();

    // 倒序遍历插入，因为 insertMessageFront 是插到由上往下的第0行
    // 比如数据是 [A, B, C]，我们先插 C 到顶，再插 B 到顶，再插 A 到顶 -> 结果界面是 A, B, C
    for (int i = start; i >= end; --i) {
        const MessageData &msg = m_allMessages.at(i);
        m_chatView->insertMessageFront(msg);
    }

    m_displayedCount += (start - end + 1);

    // 【视觉维持】加载历史后，滚动条会自动变长，我们要保持用户看到的“当前条”不动
    if (oldScrollHeight > 0) {
        int newScrollHeight = m_chatView->verticalScrollBar()->maximum();
        // 保持相对位置：往下滚一段距离，这段距离等于(新高度 - 旧高度)
        m_chatView->verticalScrollBar()->setValue(newScrollHeight - oldScrollHeight);
    }
}
void ChatWindow::onLoadMoreHistory()
{
    qDebug() << "Trigger loading history...";
    // 加个小延时模拟网络请求，或者防止滚动过快触发多次
    QTimer::singleShot(200, this, [this](){
        renderHistoryBatch();
    });
}

void ChatWindow::onSendClicked()
{
    // 发送逻辑
    QString text = ""; // 获取输入框文本 ui->inputEdit->toPlainText();
    if (text.isEmpty()) return;
    m_inputEdit->clear();

    // 构造消息
    MessageData newMsg(text, MsgRole::Self);

    // 更新内存数据 (保持一致性)
    m_allMessages.append(newMsg);
    m_displayedCount++;
    m_chatView->appendMessage(newMsg);

    // TODO: 发送给服务器
}

