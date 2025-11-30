#include "network_worker.h"
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
    if (m_initialized) {
        return true;
    }

    m_socketClient = new SocketClient(this);

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
    emit initialized();
    emit statusChanged("Network Worker initialized");
    
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
        emit errorOccurred("NetworkWorker not initialized");
        return;
    }

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
    // 修复：提取消息 ID，用于回调
    QString messageId = message.value("messageId").toString();

    // 修复：检查连接状态
    if (!m_initialized) {
        qWarning() << "NetworkWorker not initialized";
        emit messageSendFailed(messageId, "NetworkWorker not initialized");
        return;
    }

    if (!m_isConnected) {
        qWarning() << "Not connected to server";
        emit messageSendFailed(messageId, "Not connected to server");
        return;
    }

    QJsonDocument doc(message);
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    qDebug() << "Sending JSON message:" << jsonStr;
    try {
        m_socketClient->sendMessageToServer(0, jsonStr);

        // 修复：发送成功后发出信号
        emit messageSendSuccess(messageId);

    }
    catch (const std::exception& e) {
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
        emit errorOccurred("NetworkWorker not initialized");
        return;
    }

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
    qDebug() << "Message received from" << from << ":" << message;
    
    // 尝试解析为 JSON
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        emit messageReceived(doc.object(), from);
    } else {
        // 如果不是 JSON，作为文本消息发送
        emit textMessageReceived(message, from);
    }
}

void NetworkWorker::onSocketError(const QString& error)
{
    qDebug() << "Socket error:" << error;
    emit errorOccurred(error);
}
