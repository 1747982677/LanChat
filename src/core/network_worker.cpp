#include "network_worker.h"
#include "utils/logger.h"
#include <QJsonDocument>
#include <QDebug>
#include <QUuid>
#include <QDateTime>

NetworkWorker::NetworkWorker(QObject* parent)
    : BaseWorker(parent)
    , m_chatService(nullptr)
    , m_isConnected(false)
{
}

NetworkWorker::~NetworkWorker()
{
    cleanup();
}

bool NetworkWorker::initialize()
{
    Logger::getInstance().log("[NetworkWorker] initialize() called");
    if (m_initialized) {
        Logger::getInstance().log("[NetworkWorker] Already initialized");
        return true;
    }

    m_initialized = true;
    Logger::getInstance().log("[NetworkWorker] Emitting initialized signal");
    emit initialized();
    emit statusChanged("Network Worker initialized");
    
    Logger::getInstance().log("[NetworkWorker] *** Initialization complete ***");
    qDebug() << "NetworkWorker initialized";
    return true;
}

void NetworkWorker::initializeChatService(const QString& UserId)
{
	// 设置当前用户唯一ID
	m_currentUserId = UserId;
    // 获取 ChatService 单例
    m_chatService = &ChatService::getInstance();
    Logger::getInstance().log("[NetworkWorker] ChatService instance obtained");
    
    
	// 初始化 ChatService（必须先调用 initialize初始化自己的用户名）再自动发现
    if (!m_chatService->isInitialized()) {
        if (!m_chatService->initialize(m_currentUserId)) {
            Logger::getInstance().error("[NetworkWorker] Failed to initialize ChatService");
            return;
        }
        Logger::getInstance().log(QString("[NetworkWorker] ChatService initialized with user: %1").arg(m_currentUserId));
    }
    
    if (!m_chatService) {
        Logger::getInstance().error("[NetworkWorker] ChatService is null, cannot connect signals");
        return;
    }

    // 连接 ChatService 的信号
    connect(m_chatService, &ChatService::messageSent,
        this, &NetworkWorker::onChatServiceMessageSent);
    connect(m_chatService, &ChatService::messageReceived,
        this, &NetworkWorker::onChatServiceMessageReceived);
    connect(m_chatService, &ChatService::errorOccurred,
        this, &NetworkWorker::onChatServiceError);
    connect(m_chatService, &ChatService::onlineUsersUpdated,
        this, &NetworkWorker::onChatServiceOnlineUsersUpdated);

	m_chatService->autoInit(8080, 3000); // 使用自动发现，端口8080，超时3000ms
	emit connected();
    Logger::getInstance().log("[NetworkWorker] ChatService initialized and connected");
}


void NetworkWorker::cleanup()
{
    if (!m_initialized) {
        return;
    }
    m_chatService = nullptr;

    m_initialized = false;
    qDebug() << "NetworkWorker cleaned up";
}


void NetworkWorker::disconnectFromServer()
{
    // ChatService 没有直接的断开方法，但我们可以标记为未连接
    m_isConnected = false;
    m_chatService = nullptr;
    emit disconnected();
    emit connectionStateChanged(false);
}

void NetworkWorker::sendMessage(const LanChat::Message& message)
{
    // 日志增强：输出消息ID（提前获取，方便定位问题）
    QString messageId = message.messageId; // 直接访问Message对象的messageId成员
    Logger::getInstance().log(QString("[NetworkWorker] sendMessage called, messageId: %1").arg(messageId.isEmpty() ? "unknown" : messageId));

    // 1. 初始化状态检查
    if (!m_initialized || !m_chatService) {
        QString errorMsg = "NetworkWorker not initialized";
        Logger::getInstance().error(QString("[NetworkWorker] %1, messageId: %2").arg(errorMsg).arg(messageId));
        emit errorOccurred(errorMsg);

        if (!messageId.isEmpty()) {
            emit messageSendFailed(messageId, errorMsg);
        }
        return;
    }

    // 2. 必要字段验证 补全发送者ID（如果为空）
    if (message.senderId.isEmpty()) {
        //message.senderId = m_currentUserId;
        QString errorMsg = "No receiver";
        Logger::getInstance().log(QString("[NetworkWorker] Auto-filled senderId: %1 for messageId: %2").arg(m_currentUserId).arg(messageId));
        emit messageSendFailed(messageId, errorMsg);
        return;
    }

    // 校验接收者ID
    if (message.receiverId.isEmpty()) {
        QString errorMsg = "No receiverId specified in message";
        Logger::getInstance().error(QString("[NetworkWorker] %1, messageId: %2").arg(errorMsg).arg(messageId));
        if (!messageId.isEmpty()) {
            emit messageSendFailed(messageId, errorMsg);
        }
        return;
    }

    // 3. 发送消息核心逻辑
    try {
        Logger::getInstance().log(QString("[NetworkWorker] Sending message via ChatService, messageId: %1, sender: %2, receiver: %3")
            .arg(messageId).arg(message.senderId).arg(message.receiverId));
        qDebug() << "Sending message via ChatService:" << messageId
            << ", sender:" << message.senderId
            << ", receiver:" << message.receiverId;

        // 直接发送Message对象（移除冗余的JSON转换）
        m_chatService->sendMessage(message);

        Logger::getInstance().log(QString("[NetworkWorker] Message sent to ChatService successfully, messageId: %1").arg(messageId));

        // 注意：实际的发送成功信号由 ChatService 的 messageSent 信号触发
    }
    catch (const std::exception& e) {
        QString errorDetail = QString("Exception: %1").arg(e.what());
        qWarning() << "Failed to send message, messageId:" << messageId << ", error:" << e.what();

        if (!messageId.isEmpty()) {
            emit messageSendFailed(messageId, errorDetail);
        }
    }
    catch (...) {
        // 捕获所有未知异常，避免程序崩溃
        QString errorDetail = "Unknown exception occurred";
        qWarning() << "Failed to send message (unknown error), messageId:" << messageId;

        if (!messageId.isEmpty()) {
            emit messageSendFailed(messageId, errorDetail);
        }
    }
}

void NetworkWorker::sendTextMessage(const QString& text, const QString& receiverId)
{
    if (!m_initialized || !m_chatService) {
        emit errorOccurred("NetworkWorker not initialized");
        return;
    }

    Logger::getInstance().log(QString("[NetworkWorker] Sending text message: %1").arg(text));
    qDebug() << "Sending text message:" << text;

    // 创建一个简单的消息对象
    LanChat::Message msg;
    msg.messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.senderId = m_currentUserId;
    msg.content = text;
    msg.receiverId = receiverId;
    msg.type = LanChat::MessageType::Text;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();

    sendMessage(msg);
}

void NetworkWorker::stopServer()
{
    // ChatService 没有明确的停止服务器方法
    // 标记为未连接
    m_isConnected = false;
    emit disconnected();
    emit connectionStateChanged(false);
}

// ChatService 信号处理
void NetworkWorker::onChatServiceMessageSent(const LanChat::Message& message)
{
    Logger::getInstance().log(QString("[NetworkWorker] ChatService message sent: %1").arg(message.messageId));
    
    // 将 ChatService 的消息发送事件转发到上层
    if (!message.messageId.isEmpty()) {
        emit messageSendSuccess(message.messageId);
    }
}

void NetworkWorker::onChatServiceMessageReceived(const LanChat::Message& message)
{
    Logger::getInstance().log(QString("[NetworkWorker] ChatService message received from %1: %2")
        .arg(message.senderId).arg(message.content));
    
    emit messageReceived(message);
}

void NetworkWorker::onChatServiceError(const QString& error)
{
    Logger::getInstance().error(QString("[NetworkWorker] ChatService error: %1").arg(error));
    emit errorOccurred(error);
}

void NetworkWorker::onChatServiceOnlineUsersUpdated(const QStringList& userIds)
{
    Logger::getInstance().log(QString("[NetworkWorker] Online users updated: %1 users").arg(userIds.size()));
    qDebug() << "Online users:" << userIds;
    // ToDo :可以在这里添加转发在线用户列表的逻辑
    //emit onlineUsersUpdated();
}
