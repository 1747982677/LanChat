#include "network_controller.h"
#include "network_worker.h"
#include "utils/logger.h"
#include <QDebug>
#include <QJsonDocument>

NetworkController* NetworkController::s_instance = nullptr;

NetworkController::NetworkController(QObject* parent)
    : BaseController(parent)
{
}

NetworkController::~NetworkController()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

NetworkController& NetworkController::instance()
{
    if (!s_instance) {
        s_instance = new NetworkController();
    }
    return *s_instance;
}

bool NetworkController::initialize()
{
    // 创建 Worker
    QObject* worker = createWorker();
    if (!worker) {
        emit errorOccurred("Failed to create NetworkWorker");
        return false;
    }

    qDebug() << "NetworkController initialized";
    emit initialized();

    // 设置 Worker 线程
    setupWorkerThread(worker);

    // 连接信号
    connectSignals();

    // 注意：Worker 的 initialize() 将在 start() 后调用（线程启动后）
    QMetaObject::invokeMethod(worker, "initialize", Qt::QueuedConnection);
    return true;
}

QObject* NetworkController::createWorker()
{
    return new NetworkWorker();
}

void NetworkController::start()
{
    // 先调用父类的 start()，启动线程
    BaseController::start();
    
    // 线程启动后，初始化 Worker
    if (m_worker) {
        Logger::getInstance().log("[NetworkController] Thread started, initializing worker...");
        bool invoked = QMetaObject::invokeMethod(m_worker, "initialize", Qt::BlockingQueuedConnection);
        Logger::getInstance().log(QString("[NetworkController] Worker initialization invoked: %1").arg(invoked));
    }
}

void NetworkController::connectSignals()
{
    if (!m_worker) {
        return;
    }

    NetworkWorker* worker = qobject_cast<NetworkWorker*>(m_worker);
    if (!worker) {
        return;
    }

    // Controller -> Worker 信号
    connect(this, &NetworkController::requestInitializeWithUserId,
            worker, &NetworkWorker::initializeChatService);
    connect(this, &NetworkController::requestSendMessage,
            worker, &NetworkWorker::sendMessage);
	connect(this, &NetworkController::requestSendJsonMessage,
		worker, &NetworkWorker::sendJsonMessage);
    connect(this, &NetworkController::requestSendTextMessage,
            worker, &NetworkWorker::sendTextMessage);
	connect(this, &NetworkController::requestDisconnect,
		worker, &NetworkWorker::disconnectFromServer);
	connect(this, &NetworkController::requestStopServer,
		worker, &NetworkWorker::stopServer);
    connect(this, &NetworkController::requestOnlineUsers,
		worker, &NetworkWorker::requestOnlineUsers);

    // Worker -> Controller 信号（转发）
    connect(worker, &NetworkWorker::connected,
            this, &NetworkController::connected);
    connect(worker, &NetworkWorker::disconnected,
            this, &NetworkController::disconnected);
    connect(worker, &NetworkWorker::messageReceived, 
            this, &NetworkController::onmessageReceivedFromWorker);
    connect(worker, &NetworkWorker::jsonMessageReceived,
		this, &NetworkController::jsonMessageReceived);
    connect(worker, &NetworkWorker::textMessageReceived,
            this, &NetworkController::textMessageReceived);
    connect(worker, &NetworkWorker::connectionStateChanged,
            this, &NetworkController::connectionStateChanged);
    connect(worker, &NetworkWorker::messageSendSuccess,
            this, &NetworkController::messageSendSuccess);
    connect(worker, &NetworkWorker::messageSendFailed,
            this, &NetworkController::messageSendFailed);
    connect(worker, &NetworkWorker::onlineUsersUpdated,
            this, &NetworkController::onlineUsersUpdated);
    connect(worker, &NetworkWorker::errorOccurred,
            this, &NetworkController::errorOccurred);
    connect(worker, &NetworkWorker::initialized,
           this, &NetworkController::initialized);
    // 🆕 转发 Worker 的状态变化
   connect(worker, &NetworkWorker::statusChanged,
            this, &NetworkController::statusChanged);
}

void NetworkController::initializeWithUserId(const QString& userId)
{
    Logger::getInstance().log(QString("[NetworkController] Initializing with user ID: %1").arg(userId));
    emit requestInitializeWithUserId(userId);
}

void NetworkController::sendMessage(const LanChat::Message& message)
{
    Logger::getInstance().log(QString("[NetworkController] sendMessage called, JSON: %1")
                             .arg(message.messageId));

    emit requestSendMessage(message);
    Logger::getInstance().log("[NetworkController] requestSendMessage signal emitted");
}

void NetworkController::sendJsonMessage(const QJsonObject& jsonMessage)
{
    Logger::getInstance().log(QString("[NetworkController] sendJsonMessage called, JSON: %1")
                             .arg(QString::fromUtf8(QJsonDocument(jsonMessage).toJson(QJsonDocument::Compact))));
    emit requestSendJsonMessage(jsonMessage);
    Logger::getInstance().log("[NetworkController] requestSendMessage signal emitted for JSON message");
}

void NetworkController::sendTextMessage(const QString& text, const QString& receiverId)
{
    Logger::getInstance().log(QString("[NetworkController] sendTextMessage called, text: %1").arg(text));
    emit requestSendTextMessage(text, receiverId);
    Logger::getInstance().log("[NetworkController] requestSendTextMessage signal emitted");
}

void NetworkController::getOnlineUsers()
{
    Logger::getInstance().log("[NetworkController] Requesting online users...");
    emit requestOnlineUsers();
}

void NetworkController::onmessageReceivedFromWorker(const LanChat::Message& message)
{
	Logger::getInstance().log(QString("[NetworkController] onmessageReceivedFromWorker called, messageId: %1")
		.arg(message.messageId));
	//消息转换为QjsonObject格式
	QJsonObject jasonMessage = message.toJson();
	emit messageReceived(jasonMessage, message.senderId);
}