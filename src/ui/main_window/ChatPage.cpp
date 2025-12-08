#include "ChatPage.h"
#include <QDateTime>
#include <QUuid>

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
    // 1. 保存当前会话
    m_currentSession = info;

    // 2. 启用输入框
    m_input->setEnabled(true);

    // 3. 分发数据给子组件
    m_title->setSessionInfo(info);
    m_chatView->setSessionInfo(info);
}

void ChatPage::onUserInputReceived(const QString& text)
{
    // 如果没有选中会话，直接返回
    if (m_currentSession.uid().isEmpty()) return;

    // 构造消息对象
    Message msg;
    msg.mid = QUuid::createUuid().toString(); // 生成唯一ID
    msg.senderId = "me"; // 假设这是自己的ID
    msg.content = text;
    msg.timestamp = QDateTime::currentDateTime();
    msg.isSelf = true;

    QString myAvatar = ""; // 替换为实际路径
    m_chatView->appendMessage(msg, myAvatar);

    // 发出信号 进行数据持久化和网络发送
    emit sigSendMessage(m_currentSession.uid(), msg);
}

QString ChatPage::currentSessionId() const
{
    return m_currentSession.uid();
}
