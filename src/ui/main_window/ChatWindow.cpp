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
	//ui.setupUi(this);
	this->setMinimumWidth(500);
	initUI();

	
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

void ChatWindow::initUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_stack = new QStackedWidget(this);
	layout->addWidget(m_stack);

	// --- 1. 创建空白页 ---
	m_emptyPage = new QWidget(this);
	m_emptyPage->setStyleSheet("background-color: #f5f5f5;");

	// 给空白页加个 Logo
	QVBoxLayout* emptyLayout = new QVBoxLayout(m_emptyPage);
	QLabel* lblIcon = new QLabel("LanChat", m_emptyPage);
	lblIcon->setAlignment(Qt::AlignCenter);
	lblIcon->setStyleSheet("font-size: 40px; color: #e0e0e0; font-weight: bold;");
	emptyLayout->addWidget(lblIcon);

	// --- 2. 创建聊天窗口 ---
	m_chatPage = new ChatPage(this);

	// --- 3. 加入堆栈 ---
	m_stack->addWidget(m_emptyPage);   // index 0
	m_stack->addWidget(m_chatPage);  // index 1

	// 初始显示空白页
	m_stack->setCurrentIndex(0);

	// --- 4. 内部信号转发 ---
	// 当 ChatWindow 发消息时，RightArea 收到后对外发射 sigSendMessage
	connect(m_chatPage, &ChatPage::sigSendMessage,
		this, &ChatWindow::sigSendMessage);
}

void ChatWindow::setSessionInfo(const SessionInfo& info)
{
	// 1. 更新 ChatWindow 数据
	m_chatPage->onSessionSelected(info);

	// 2. 如果当前还在显示空页面，切换到聊天页面
	if (m_stack->currentWidget() != m_chatPage) {
		m_stack->setCurrentWidget(m_chatPage);
	}
}