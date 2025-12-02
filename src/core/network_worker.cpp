#include "network_worker.h"
#include "utils/logger.h"
#include <QJsonDocument>
#include <QDebug>

NetworkWorker::NetworkWorker(QObject* parent)
    : BaseWorker(parent)
    , m_socketClient(nullptr)
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

    m_socketClient = new SocketClient(this);
    Logger::getInstance().log("[NetworkWorker] SocketClient created");

    // 连接信号
    connect(m_socketClient, &SocketClient::connected,
            this, &NetworkWorker::onSocketConnected);
    connect(m_socketClient, &SocketClient::disconnected,
            this, &NetworkWorker::onSocketDisconnected);
    connect(m_socketClient, &SocketClient::messageReceived,
            this, &NetworkWorker::onSocketMessageReceived);
    connect(m_socketClient, &SocketClient::errorOccurred,
            this, &NetworkWorker::onSocketError);

    m_initialized = true;
    Logger::getInstance().log("[NetworkWorker] Emitting initialized signal");
    emit initialized();
    emit statusChanged("Network Worker initialized");
    
    Logger::getInstance().log("[NetworkWorker] *** Initialization complete ***");
    qDebug() << "NetworkWorker initialized";
    return true;
}

void NetworkWorker::cleanup()
{
    if (!m_initialized) {
        return;
    }

    if (m_socketClient) {
        m_socketClient->disconnect();
        m_socketClient->stopServer();
        m_socketClient->deleteLater();
        m_socketClient = nullptr;
    }

    m_initialized = false;
    qDebug() << "NetworkWorker cleaned up";
}

void NetworkWorker::connectToServer(const QString& host, quint16 port)
{
    if (!m_initialized) {
        Logger::getInstance().error("[NetworkWorker] Not initialized when connecting");
        emit errorOccurred("NetworkWorker not initialized");
        return;
    }

    Logger::getInstance().log(QString("[NetworkWorker] Connecting to %1:%2").arg(host).arg(port));
    qDebug() << "Connecting to" << host << ":" << port;
    m_socketClient->connectToHost(host, port);
}

void NetworkWorker::disconnectFromServer()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Disconnecting from server";
    m_socketClient->disconnect();
}

void NetworkWorker::sendMessage(const QJsonObject& message)
{
    Logger::getInstance().log("[NetworkWorker] sendMessage called");
    if (!m_initialized || !m_isConnected) {
        Logger::getInstance().error("[NetworkWorker] Not connected to server");
        emit errorOccurred("Not connected to server");
        return;
    }

    // 提取消息 ID（如果有）
    QString messageId = message.value("messageId").toString();

    // 将 QJsonObject 转换为 JSON 文档
    QJsonDocument doc(message);
    // 将 QJsonObject 转换为紧凑的 JSON 字符串（字节流）
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    Logger::getInstance().log(QString("[NetworkWorker] Sending JSON message: %1").arg(jsonStr));
    qDebug() << "Sending JSON message:" << jsonStr;
    try {
        // 修复：发送消息到服务器
        m_socketClient->sendMessageToServer(0, jsonStr);
        Logger::getInstance().log("[NetworkWorker] Message sent to SocketClient");

        // 修复：发送成功后发出信号
        emit messageSendSuccess(messageId);

    }
    catch (const std::exception& e) {
        Logger::getInstance().error(QString("[NetworkWorker] Failed to send message: %1").arg(e.what()));
        qWarning() << "Failed to send message:" << e.what();
        emit messageSendFailed(messageId, QString("Exception: %1").arg(e.what()));
    }
}

void NetworkWorker::sendTextMessage(const QString& text)
{
    if (!m_initialized || !m_isConnected) {
        emit errorOccurred("Not connected to server");
        return;
    }

    qDebug() << "Sending text message:" << text;
    m_socketClient->sendMessageToServer(0, text);
}

void NetworkWorker::startServer(quint16 port)
{
    if (!m_initialized) {
        Logger::getInstance().error("[NetworkWorker] Not initialized when starting server");
        emit errorOccurred("NetworkWorker not initialized");
        return;
    }

    Logger::getInstance().log(QString("[NetworkWorker] Starting server on port %1").arg(port));
    qDebug() << "Starting server on port" << port;
    m_socketClient->startServer(port);
}

void NetworkWorker::stopServer()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Stopping server";
    m_socketClient->stopServer();
}

void NetworkWorker::onSocketConnected()
{
    m_isConnected = true;
    qDebug() << "Socket connected";
    emit connected();
    emit connectionStateChanged(true);
}

void NetworkWorker::onSocketDisconnected()
{
    m_isConnected = false;
    qDebug() << "Socket disconnected";
    emit disconnected();
    emit connectionStateChanged(false);
}

void NetworkWorker::onSocketMessageReceived(const QString& message, const QString& from)
{
    Logger::getInstance().log(QString("[NetworkWorker] *** Message received from %1: %2").arg(from).arg(message));
    qDebug() << "Message received from" << from << ":" << message;
    
    // 尝试解析为 JSON
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        Logger::getInstance().log("[NetworkWorker] Parsed as JSON, emitting messageReceived signal");
        emit messageReceived(doc.object(), from);
    } else {
        Logger::getInstance().log("[NetworkWorker] Not JSON, emitting textMessageReceived signal");
        // 如果不是 JSON，作为文本消息发送
        emit textMessageReceived(message, from);
    }
}

void NetworkWorker::onSocketError(const QString& error)
{
    qDebug() << "Socket error:" << error;
    emit errorOccurred(error);
}
