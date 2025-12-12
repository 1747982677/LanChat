#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include "SessionInfo.h"
#include "ChatTitle.h"
#include "ChatInputWidget.h"
#include "NChatView.h"
#include "model/message_dao.h"
#include "service/auth_service.h"


class ChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPage(QWidget* parent = nullptr);

    // 获取当前会话ID
    QString currentSessionId() const;
    // 追加一条来自对方的消息（需当前会话匹配）
    void appendIncomingMessage(const UiMessage& msg);

public slots:
    // 【核心槽函数】响应左侧列表点击，切换会话
    void onSessionSelected(const SessionInfo& info);

signals:
    // 当用户发送消息时，向外发出信号（通知 Manager 更新数据）
    void sigSendMessage(const QString& targetUid, const UiMessage& msg);

private slots:
    // 内部槽：处理输入框发来的文本
    void onUserInputReceived(const QString& text);

private:
    void initUI();

    ChatTitle* m_title;          // 顶部标题
    NChatView* m_chatView;       // 中间消息列表
    ChatInputWidget* m_input;    // 底部输入框

    SessionInfo m_currentSession; // 当前正在聊天的会话数据
};
