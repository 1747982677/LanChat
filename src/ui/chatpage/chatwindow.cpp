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
#include <QUuid>
#include <QDateTime>
#include "core/network_controller.h"
#include "core/app_context.h"
#include "service/auth_service.h"
#include "common/types.h"
#include "utils/logger.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
    , m_currentUserId("")
    , m_displayedCount(0)
{
    Logger::getInstance().log("[ChatWindow] Constructor started");
    // 主布局（不使用 UI 文件，完全用代码构建）
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

    // 在调试模式下，如果当前没有选中的用户，初始化一个测试会话，便于本地自测
// #ifdef QT_DEBUG
//     if (m_currentUserId.isEmpty()) {
//         const QString testUserId = "test_peer";
//         const QString testUserName = "测试用户";
//         Logger::getInstance().log(QString("[ChatWindow] QT_DEBUG: initializing default test chat with %1 (%2)")
//                                   .arg(testUserId).arg(testUserName));
//         switchChat(testUserId, testUserName);
//     }
// #endif

    // 连接网络消息接收信号（通过 AppContext 获取指针）
    NetworkController* netCtrl = AppContext::instance().networkController();
    Logger::getInstance().log(QString("[ChatWindow] NetworkController pointer: %1").arg((quintptr)netCtrl, 0, 16));
    
    bool connected = connect(netCtrl, &NetworkController::messageReceived,
                             this, &ChatWindow::onMessageReceived);
    Logger::getInstance().log(QString("[ChatWindow] Signal connection result: %1").arg(connected ? "SUCCESS" : "FAILED"));
    
    if (connected) {
        Logger::getInstance().log("[ChatWindow] Signal connected to NetworkController");
    } else {
        Logger::getInstance().error("[ChatWindow] FAILED to connect signal!");
    }
}

ChatWindow::~ChatWindow()
{
    Logger::getInstance().log("[ChatWindow] Destructor called");
    delete ui;
}

// 【核心逻辑】切换用户
void ChatWindow::switchChat(const QString &userId, const QString &userName)
{
    Logger::getInstance().log(QString("[ChatWindow] switchChat called: %1, %2").arg(userId).arg(userName));
    // 如果点的就是当前这人，啥都不做
    if (userId == m_currentUserId) return;

    // 直接更新当前聊天对象 ID
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
//UI层处理发送按钮点击事件
void ChatWindow::onSendClicked()
{
    // 获取输入框文本
    QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    // 清空输入框
    m_inputEdit->clear();

    // 1. 立即在 UI 上显示（提升用户体验）
    MessageData uiMsg(text, MsgRole::Self);
    m_allMessages.append(uiMsg);
    m_displayedCount++;
    m_chatView->appendMessage(uiMsg);
    m_chatView->scrollToBottom();

    // 2. 构造网络消息对象
    LanChat::Message msg;
    msg.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // 使用 AuthService 提供的当前登录用户 ID 作为发送者
    QString currentUserId = AuthService::getInstance().getCurrentUserId();
    msg.senderId = currentUserId;
    msg.receiverId = m_currentUserId;
    msg.content = text;
    msg.type = LanChat::MessageType::Text;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.status = LanChat::MessageStatus::Pending;

    // 3. 发送到网络层
    // 通过 AppContext 获取全局唯一的 NetworkController 实例，保持与构造函数中连接信号的是同一个对象
    NetworkController* netCtrl = AppContext::instance().networkController();
    if (netCtrl) {
        netCtrl->sendMessage(msg);
    } else {
        Logger::getInstance().error("[ChatWindow] NetworkController is null, cannot send message");
    }
    Logger::getInstance().log(QString("[ChatWindow] Sending message from %1 to %2: %3")
                             .arg(msg.senderId).arg(m_currentUserId).arg(text));
}
//UI层处理网络层接收到的消息
void ChatWindow::onMessageReceived(const QJsonObject &msgJson, const QString &from)
{
    Logger::getInstance().log(QString("[ChatWindow] *** onMessageReceived CALLED! from: %1").arg(from));
    Q_UNUSED(from);

    // 1. 反序列化消息
    LanChat::Message msg = LanChat::Message::fromJson(msgJson);
    Logger::getInstance().log(QString("[ChatWindow] Parsed message - sender: %1, content: %2").arg(msg.senderId).arg(msg.content));
    
    Logger::getInstance().log(QString("[ChatWindow] Received message from %1: %2")
                             .arg(msg.senderId).arg(msg.content));

    // 2. 判断是否是当前聊天对象发来的消息
    if (msg.senderId != m_currentUserId) {
        // 不是当前聊天对象的消息，忽略（未读计数由 ChatService 处理）
        Logger::getInstance().log(QString("[ChatWindow] Message from %1 ignored (current chat: %2)")
                                 .arg(msg.senderId).arg(m_currentUserId));
        return;
    }

    // 3. 显示在聊天窗口中
    MessageData uiMsg(msg.content, MsgRole::Other);
    m_allMessages.append(uiMsg);
    m_displayedCount++;
    m_chatView->appendMessage(uiMsg);
    m_chatView->scrollToBottom();
}

