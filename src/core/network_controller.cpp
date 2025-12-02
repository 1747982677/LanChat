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

    // 设置 Worker 线程
    setupWorkerThread(worker);

    // 连接信号
    connectSignals();

    // 注意：Worker 的 initialize() 将在 start() 后调用（线程启动后）
    Logger::getInstance().log("[NetworkController] Controller initialized (Worker will be initialized after thread starts)");
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
    connect(this, &NetworkController::requestConnect,
            worker, &NetworkWorker::connectToServer);
    connect(this, &NetworkController::requestDisconnect,
            worker, &NetworkWorker::disconnectFromServer);
    connect(this, &NetworkController::requestSendMessage,
            worker, &NetworkWorker::sendMessage);
    connect(this, &NetworkController::requestSendTextMessage,
            worker, &NetworkWorker::sendTextMessage);
    connect(this, &NetworkController::requestStartServer,
            worker, &NetworkWorker::startServer);
    connect(this, &NetworkController::requestStopServer,
            worker, &NetworkWorker::stopServer);

    // Worker -> Controller 信号（转发）
    connect(worker, &NetworkWorker::connected,
            this, &NetworkController::connected);
    connect(worker, &NetworkWorker::disconnected,
            this, &NetworkController::disconnected);
    connect(worker, &NetworkWorker::messageReceived,
            this, [this](const QJsonObject& msg, const QString& from) {
                Logger::getInstance().log(QString("[NetworkController] messageReceived from worker, forwarding signal. from: %1").arg(from));
                emit messageReceived(msg, from);
            });
    connect(worker, &NetworkWorker::textMessageReceived,
            this, &NetworkController::textMessageReceived);
    connect(worker, &NetworkWorker::connectionStateChanged,
            this, &NetworkController::connectionStateChanged);
    connect(worker, &NetworkWorker::errorOccurred,
            this, &NetworkController::errorOccurred);

    // 新增：转发消息发送结果
    connect(worker, &NetworkWorker::messageSendSuccess,
        this, &NetworkController::messageSendSuccess);
    connect(worker, &NetworkWorker::messageSendFailed,
        this, &NetworkController::messageSendFailed);
    
    // 转发 Worker 的 initialized 信号
    connect(worker, &NetworkWorker::initialized,
        this, [this](){
            Logger::getInstance().log("[NetworkController] Worker initialized, emitting controller initialized signal");
            emit initialized();
        });
    
    Logger::getInstance().log("[NetworkController] All signals connected");
}

void NetworkController::connectToServer(const QString& host, quint16 port)
{
    qDebug() << "NetworkController: Request connect to" << host << ":" << port;
    emit requestConnect(host, port);
}

void NetworkController::disconnectFromServer()
{
    qDebug() << "NetworkController: Request disconnect";
    Logger::getInstance().log("[NetworkController] disconnectFromServer called");
    emit requestDisconnect();
}

void NetworkController::sendMessage(const QJsonObject& message)
{
    Logger::getInstance().log(QString("[NetworkController] sendMessage called, JSON: %1")
                             .arg(QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact))));
    emit requestSendMessage(message);
    Logger::getInstance().log("[NetworkController] requestSendMessage signal emitted");
}

void NetworkController::sendTextMessage(const QString& text)
{
    Logger::getInstance().log(QString("[NetworkController] sendTextMessage called, text: %1").arg(text));
    emit requestSendTextMessage(text);
    Logger::getInstance().log("[NetworkController] requestSendTextMessage signal emitted");
}

void NetworkController::startServer(quint16 port)
{
    qDebug() << "NetworkController: Request start server on port" << port;
    emit requestStartServer(port);
}

void NetworkController::stopServer()
{
    qDebug() << "NetworkController: Request stop server";
    emit requestStopServer();
}
