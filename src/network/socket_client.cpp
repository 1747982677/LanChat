#include "socket_client.h"
#include "utils/logger.h"
#include <QUrl>

SocketClient::SocketClient(QObject *parent)
    : QObject(parent),
      m_socket(nullptr),
      m_serverPort(0),
      m_heartbeatInterval(10000),
      m_missedPongs(0),
      m_maxReconnectAttempts(10),
      m_reconnectAttempts(0),
      m_isConnected(false)
{
    // 创建 WebSocket
    m_socket = new QWebSocket();
    
    // 连接信号
    connect(m_socket, &QWebSocket::connected, this, &SocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &SocketClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &SocketClient::onTextMessageReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &SocketClient::onError);
    connect(m_socket, &QWebSocket::pong, this, &SocketClient::onPong);

    // 创建心跳定时器
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(m_heartbeatInterval);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &SocketClient::onHeartbeatTimeout);

    // 创建重连定时器
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &SocketClient::onReconnectTimeout);
}

SocketClient::~SocketClient()
{
    if (m_socket) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->close();
        }
        m_socket->deleteLater();
    }
}

void SocketClient::connectToServer(const QString &host, quint16 port)
{
    if (m_isConnected || m_socket->state() == QAbstractSocket::ConnectingState) {
        Logger::getInstance().log(QString("Already connected or connecting to server"));
        return;
    }

    m_serverHost = host;
    m_serverPort = port;
    m_reconnectAttempts = 0;

    QString url = QString("ws://%1:%2").arg(host).arg(port);
    Logger::getInstance().log(QString("Connecting to central server: %1").arg(url));
    m_socket->open(QUrl(url));
}

void SocketClient::disconnectFromServer()
{
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    
    stopHeartbeat();
    
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->close();
    }
    
    m_isConnected = false;
    Logger::getInstance().log("Disconnected from server");
}

void SocketClient::sendMessage(const QString &message)
{
    if (!m_isConnected || m_socket->state() != QAbstractSocket::ConnectedState) {
        Logger::getInstance().error("Cannot send message: not connected to server");
        emit errorOccurred("Not connected to server");
        return;
    }

    m_socket->sendTextMessage(message);
    Logger::getInstance().log(QString("Sent message to server: %1").arg(message));
}

bool SocketClient::isConnected() const
{
    return m_isConnected && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString SocketClient::getServerAddress() const
{
    if (!m_serverHost.isEmpty() && m_serverPort > 0) {
        return QString("%1:%2").arg(m_serverHost).arg(m_serverPort);
    }
    return QString();
}

void SocketClient::setMaxReconnectAttempts(int attempts)
{
    m_maxReconnectAttempts = attempts;
    Logger::getInstance().log(QString("Max reconnect attempts set to %1").arg(attempts));
}

int SocketClient::getMaxReconnectAttempts() const
{
    return m_maxReconnectAttempts;
}

void SocketClient::setHeartbeatInterval(int intervalMs)
{
    m_heartbeatInterval = intervalMs;
    m_heartbeatTimer->setInterval(intervalMs);
    Logger::getInstance().log(QString("Heartbeat interval set to %1 ms").arg(intervalMs));
}

// -------------------- Private Slots --------------------

void SocketClient::onConnected()
{
    m_isConnected = true;
    m_reconnectAttempts = 0;
    m_missedPongs = 0;
    
    QString addr = getServerAddress();
    Logger::getInstance().log(QString("Connected to central server: %1").arg(addr));
    
    startHeartbeat();
    
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    
    emit connected();
    emit connectionStateChanged(true);
}

void SocketClient::onDisconnected()
{
    bool wasConnected = m_isConnected;
    m_isConnected = false;
    
    QString addr = getServerAddress();
    Logger::getInstance().log(QString("Disconnected from central server: %1").arg(addr));
    
    stopHeartbeat();
    
    // 自动重连
    if (wasConnected) {
        scheduleReconnect();
    }
    
    emit disconnected();
    emit connectionStateChanged(false);
}

void SocketClient::onTextMessageReceived(const QString &message)
{
    Logger::getInstance().log(QString("Message received from server: %1").arg(message));
    
    // 收到消息，重置 pong 计数
    m_missedPongs = 0;
    
    emit messageReceived(message);
}

void SocketClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString err = m_socket->errorString();
    Logger::getInstance().error(QString("Client error: %1").arg(err));
    emit errorOccurred(err);
}

void SocketClient::onPong(quint64 elapsedTime)
{
    Q_UNUSED(elapsedTime);
    m_missedPongs = 0;
    Logger::getInstance().log("Received pong from server");
}

void SocketClient::onHeartbeatTimeout()
{
    if (!m_isConnected || m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    // 发送 ping
    m_socket->ping();
    m_missedPongs++;
    
    Logger::getInstance().log(QString("Sent ping to server (missed pongs: %1)").arg(m_missedPongs));
    
    // 检查是否超过 3 次未收到 pong
    if (m_missedPongs >= 3) {
        Logger::getInstance().error("Missed 3 pongs from server, closing connection");
        m_socket->close();
    }
}

void SocketClient::onReconnectTimeout()
{
    m_reconnectAttempts++;
    
    // 检查是否超过最大重连次数
    if (m_maxReconnectAttempts > 0 && m_reconnectAttempts > m_maxReconnectAttempts) {
        QString addr = getServerAddress();
        Logger::getInstance().error(QString("Max reconnect attempts (%1) reached for %2, stopping reconnect")
            .arg(m_maxReconnectAttempts).arg(addr));
        emit errorOccurred(QString("Reconnect failed after %1 attempts").arg(m_reconnectAttempts));
        return;
    }

    QString url = QString("ws://%1:%2").arg(m_serverHost).arg(m_serverPort);
    Logger::getInstance().log(QString("Reconnect attempt %1 to %2").arg(m_reconnectAttempts).arg(url));
    m_socket->open(QUrl(url));
}

// -------------------- Private Methods --------------------

void SocketClient::startHeartbeat()
{
    if (!m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start();
        Logger::getInstance().log("Started heartbeat");
    }
}

void SocketClient::stopHeartbeat()
{
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->stop();
        Logger::getInstance().log("Stopped heartbeat");
    }
}

void SocketClient::scheduleReconnect()
{
    // 检查是否已达到最大重连次数
    if (m_maxReconnectAttempts > 0 && m_reconnectAttempts >= m_maxReconnectAttempts) {
        QString addr = getServerAddress();
        Logger::getInstance().error(QString("Max reconnect attempts reached for %1").arg(addr));
        emit errorOccurred(QString("Reconnect failed after %1 attempts").arg(m_reconnectAttempts));
        return;
    }

    // 指数退避算法计算延迟时间
    int shift = qMin(m_reconnectAttempts, 6);
    int base = 1000;
    int cap = 30000;
    int delay = qMin(cap, base * (1 << shift));
    
    QString addr = getServerAddress();
    Logger::getInstance().log(QString("Scheduling reconnect to %1 in %2 ms (attempt %3)")
        .arg(addr).arg(delay).arg(m_reconnectAttempts + 1));
    
    m_reconnectTimer->start(delay);
}

