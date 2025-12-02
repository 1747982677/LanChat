// ChatWindow.cpp
/*
 * 页面名称：聊天页面
 * 功能：显示当前用户的聊天会话
 */
#include "ChatWindow.h"
#include "core/network_controller.h"
#include "core/app_context.h"
#include "common/types.h"
#include "utils/logger.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>

//聊天框占位

ChatWindow::ChatWindow(QWidget *parent)
	: QWidget(parent)
{
	Logger::getInstance().log("[ChatWindow] Constructor started");
	ui.setupUi(this);
	this->setMinimumWidth(300);
	
	// 连接网络消息接收信号
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
}

void ChatWindow::onMessageReceived(const QJsonObject &msgJson, const QString &from)
{
	Logger::getInstance().log(QString("[ChatWindow] *** onMessageReceived CALLED! from: %1").arg(from));
	
	// 反序列化消息
	LanChat::Message msg = LanChat::Message::fromJson(msgJson);
	Logger::getInstance().log(QString("[ChatWindow] Parsed message - sender: %1, content: %2")
							 .arg(msg.senderId).arg(msg.content));
	
	// 显示弹窗验证消息接收
	QMessageBox::information(this, "收到网络消息",
							 QString("发送者: %1\n内容: %2")
							 .arg(msg.senderId).arg(msg.content));
}

