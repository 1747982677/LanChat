#include "ChatPage.h"
#include "utils/logger.h"
#include <QDateTime>
#include <QUuid>
#include <QDebug>

ChatPage::ChatPage(QWidget* parent) : QWidget(parent)
{
    initUI();
}

void ChatPage::initUI()
{
    m_title = new ChatTitle(this);
    m_chatView = new NChatView(this);
    m_input = new ChatInputWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(m_title);
    layout->addWidget(m_chatView); // 只有它会自动拉伸
    layout->addWidget(m_input);

    m_input->setFixedHeight(150);

    // 连接输入框的信号
    connect(m_input, &ChatInputWidget::signalSendMessage,
        this, &ChatPage::onUserInputReceived);

    // 初始状态禁用输入，直到选中某个会话
    m_input->setEnabled(false);
}

void ChatPage::onSessionSelected(const SessionInfo& info)
{
    Logger::getInstance().log("[ChatPage] Session selected: " + info.uid());
    
    // 1. 保存当前会话
    m_currentSession = info;

    // 1.1 加载历史消息（从本地数据库）
    QString selfId = AuthService::getInstance().getCurrentUserId();
    QVector<Message> history = MessageDao::getConversation(selfId, info.uid(), 500);
    QList<UiMessage> uiMsgs;
    for (const Message& m : history) {
        UiMessage um;
        um.mid = QString::number(m.id);
        um.senderId = m.sender;
        um.content = m.content;
        um.timestamp = m.timestamp;
        um.isSelf = (m.sender == selfId);
        uiMsgs.append(um);
    }
    SessionInfo loaded = info;
    for (const UiMessage& um : uiMsgs) {
        loaded.addNewMessage(um);
    }
    m_currentSession = loaded;

    // 2. 启用输入框
    m_input->setEnabled(true);

    // 3. 分发数据给子组件
    m_title->setSessionInfo(m_currentSession);
    m_chatView->setSessionInfo(m_currentSession);
}

void ChatPage::onUserInputReceived(const QString& text)
{
    Logger::getInstance().log("[ChatPage] onUserInputReceived called with text: " + text);
    
    // 如果没有选中会话，直接返回
    if (m_currentSession.uid().isEmpty()) {
        Logger::getInstance().warning("[ChatPage] No session selected, ignoring message");
        return;
    }

    Logger::getInstance().log("[ChatPage] Creating message for session: " + m_currentSession.uid());
    
    // 构造消息对象
    UiMessage msg;
    msg.mid = QUuid::createUuid().toString(); // 生成唯一ID
    msg.senderId = "me"; // 假设这是自己的ID
    msg.content = text;
    msg.timestamp = QDateTime::currentDateTime();
    msg.isSelf = true;

    Logger::getInstance().log("[ChatPage] Message created with ID: " + msg.mid);
    
    QString myAvatar = ""; // 替换为实际路径
    
    // 先显示消息，这样用户能立即看到
    m_chatView->appendMessage(msg, myAvatar);
    Logger::getInstance().log("[ChatPage] Message appended to chat view");

    // 发出信号 进行数据持久化和网络发送（异步处理，不阻塞UI）
    emit sigSendMessage(m_currentSession.uid(), msg);
    Logger::getInstance().log("[ChatPage] sigSendMessage signal emitted");
}

QString ChatPage::currentSessionId() const
{
    return m_currentSession.uid();
}

void ChatPage::appendIncomingMessage(const UiMessage& msg)
{
    // 仅在当前会话匹配时展示
    if (msg.senderId != m_currentSession.uid()) {
        return;
    }
    QString avatar = m_currentSession.avatarPath();
    m_chatView->appendMessage(msg, avatar);
}
