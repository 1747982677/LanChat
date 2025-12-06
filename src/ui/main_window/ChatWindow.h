#pragma once

#include <QWidget>
#include <QJsonObject>
#include "ui_ChatWindow.h"
#include <QStackedWidget>
#include "ChatPage.h"
#include "SessionInfo.h"

class ChatWindow : public QWidget
{
	Q_OBJECT

public:
	ChatWindow(QWidget *parent = nullptr);
	~ChatWindow();

public slots:
	// 【对外接口】当左侧选中会话时调用此函数
	void setSessionInfo(const SessionInfo& info);

signals:
	// 【转发信号】聊天窗口发送消息时，转发给 MainWindow
	void sigSendMessage(const QString& targetUid, const Message& msg);

private slots:
	void onMessageReceived(const QJsonObject &msgJson, const QString &from);

private:
	//Ui::ChatWindowClass ui;

	void initUI();

	QStackedWidget* m_stack;      // 页面堆栈
	ChatPage* m_chatPage;     // 聊天窗口
	QWidget* m_emptyPage;         // 空白页
};

