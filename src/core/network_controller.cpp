#include "network_controller.h"
#include "network_worker.h"
#include <QDebug>

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

    // 初始化 Worker（需要在 Worker 线程中执行）
    QMetaObject::invokeMethod(worker, "initialize", Qt::QueuedConnection);

    qDebug() << "NetworkController initialized";
    emit initialized();
    return true;
}

QObject* NetworkController::createWorker()
{
    return new NetworkWorker();
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
            this, &NetworkController::messageReceived);
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
}

void NetworkController::connectToServer(const QString& host, quint16 port)
{
    qDebug() << "NetworkController: Request connect to" << host << ":" << port;
    emit requestConnect(host, port);
}

void NetworkController::disconnectFromServer()
{
    qDebug() << "NetworkController: Request disconnect";
    emit requestDisconnect();
}

void NetworkController::sendMessage(const QJsonObject& message)
{
    emit requestSendMessage(message);
}

void NetworkController::sendTextMessage(const QString& text)
{
    emit requestSendTextMessage(text);
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
