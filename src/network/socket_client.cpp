#include "socket_client.h"
#include "utils/logger.h"

#include <QHostAddress>
#include <QUrl>

// -------------------- Construction --------------------
SocketClient::SocketClient(QObject *parent)
    : QObject(parent), webSocketServer(nullptr), maxReconnectAttempts(10)
{
}

SocketClient::~SocketClient()
{
    // 清理客户端连接
    for (QWebSocket *s : clientSockets) {
        if (s->state() == QAbstractSocket::ConnectedState) s->close();
        // cleanup timers/resources
        cleanupClientResources(s);
        s->deleteLater();
    }
    clientSockets.clear();

    // 清理服务器
    stopServer();
}

// -------------------- Server API --------------------

bool SocketClient::startServer(quint16 port)
{
    if (webSocketServer) {
        Logger::getInstance().log("WebSocket server already running");
        return false;
    }

    webSocketServer = new QWebSocketServer(QStringLiteral("LanChat Server"),
                                           QWebSocketServer::NonSecureMode,
                                           this);
    if (!webSocketServer->listen(QHostAddress::Any, port)) {
        QString err = webSocketServer->errorString();
        Logger::getInstance().error(QString("WebSocket server start failed: %1").arg(err));
        emit serverError(err);
        delete webSocketServer;
        webSocketServer = nullptr;
        return false;
    }

    connect(webSocketServer, &QWebSocketServer::newConnection, this, &SocketClient::onNewConnection);
    Logger::getInstance().log(QString("WebSocket server started on port %1").arg(webSocketServer->serverPort()));
    emit serverStarted(webSocketServer->serverPort());
    return true;
}

void SocketClient::stopServer()
{
    if (!webSocketServer) return;
    for (QWebSocket *c : serverClients) {
        c->close();
        c->deleteLater();
    }
    serverClients.clear();
    webSocketServer->close();
    webSocketServer->deleteLater();
    webSocketServer = nullptr;
    Logger::getInstance().log("WebSocket server stopped");
    emit serverStopped();
}

bool SocketClient::isServerRunning() const
{
    return webSocketServer && webSocketServer->isListening();
}

quint16 SocketClient::getServerPort() const
{
    return (webSocketServer && webSocketServer->isListening()) ? webSocketServer->serverPort() : 0;
}

void SocketClient::sendMessageToClient(int index, const QString& message)
{
    if (!isServerRunning()) {
        Logger::getInstance().error("Cannot send: server not running");
        return;
    }
    if (index < 0 || index >= serverClients.size()) {
        Logger::getInstance().error(QString("Invalid server client index: %1").arg(index));
        return;
    }
    QWebSocket* client = serverClients.at(index);
    if (client->state() != QAbstractSocket::ConnectedState) {
        Logger::getInstance().error(QString("Server client %1 not connected").arg(getClientAddress(client)));
        return;
    }
    client->sendTextMessage(message);
    Logger::getInstance().log(QString("Sent message to server-client %1: %2").arg(getClientAddress(client)).arg(message));
}

void SocketClient::sendMessageToClientByAddress(const QString& clientAddress, const QString& message)
{
    if (!isServerRunning()) {
        Logger::getInstance().error("Cannot send: server not running");
        return;
    }
    for (QWebSocket* client : serverClients) {
        if (getClientAddress(client) == clientAddress) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->sendTextMessage(message);
                Logger::getInstance().log(QString("Sent message to server-client %1: %2").arg(clientAddress).arg(message));
                return;
            }
            else {
                Logger::getInstance().error(QString("Server client %1 not connected").arg(clientAddress));
                return;
            }
        }
    }
    Logger::getInstance().error(QString("Server client %1 not found").arg(clientAddress));
}

void SocketClient::broadcastMessage(const QString &message)
{
    if (!isServerRunning()) {
        Logger::getInstance().error("Cannot broadcast: server is not running");
        return;
    }
    int sent = 0;
    for (QWebSocket *c : serverClients) {
        if (c->state() == QAbstractSocket::ConnectedState) {
            c->sendTextMessage(message);
            ++sent;
        }
    }
    Logger::getInstance().log(QString("Broadcasted message to %1 clients").arg(sent));
}

int SocketClient::getConnectedClientCount() const
{
    return serverClients.size();
}

QStringList SocketClient::getServerClientAddresses() const
{
    QStringList list;
    for (QWebSocket *c : serverClients) list << getClientAddress(c);
    return list;
}

// -------------------- Client API (multiple) --------------------

void SocketClient::connectToHost(const QString &host, quint16 port)
{
    QWebSocket *client = new QWebSocket();
    // store host/port for reconnect
    clientHost[client] = host;
    clientPort[client] = port;

    // 心跳定时器
    QTimer *hb = new QTimer(this);
    hb->setInterval(10000); // 10s
    heartbeatTimers[client] = hb;

    // 重连定时器
    QTimer *rt = new QTimer(this);
    rt->setSingleShot(true);
    reconnectTimers[client] = rt;
    reconnectAttempts[client] = 0;
    missedPongs[client] = 0;

    connect(client, &QWebSocket::connected, this, &SocketClient::onClientConnected);
    connect(client, &QWebSocket::disconnected, this, &SocketClient::onClientDisconnected);
    connect(client, &QWebSocket::textMessageReceived, this, &SocketClient::onClientTextMessageReceived);
    connect(client, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this, &SocketClient::onClientError);

    // pong信号处理
    connect(client, &QWebSocket::pong, this, [this, client](quint64){
        missedPongs[client] = 0;
    });

    // heartbeat timeout -> ping and check missed pongs
    connect(hb, &QTimer::timeout, this, [this, client]() {
        if (!client) return;
        if (client->state() != QAbstractSocket::ConnectedState) return;
        // send ping
        client->ping();
        missedPongs[client]++;
        if (missedPongs[client] >= 3) {
            Logger::getInstance().error("Missed pongs, closing client and scheduling reconnect");
            client->close();
            // 会触发重连
        }
    });

    // reconnect timer timeout -> attempt reconnect
    connect(rt, &QTimer::timeout, this, [this, client]() {
        int attempts = reconnectAttempts.value(client, 0) + 1;
        reconnectAttempts[client] = attempts;
        // check max attempts before trying
        if (maxReconnectAttempts > 0 && attempts > maxReconnectAttempts) {
            QString addr = getClientAddress(client);
            Logger::getInstance().error(QString("Max reconnect attempts reached for %1, stopping reconnect").arg(addr));
            // cleanup and remove client
            cleanupClientResources(client);
            clientSockets.removeAll(client);
            client->deleteLater();
            emit errorOccurred(QString("Reconnect failed for %1 after %2 attempts").arg(addr).arg(attempts));
            return;
        }

        QString host = clientHost.value(client);
        quint16 port = clientPort.value(client);
        QString url = QString("ws://%1:%2").arg(host).arg(port);
        Logger::getInstance().log(QString("Reconnect attempt %1 to %2").arg(attempts).arg(url));
        client->open(QUrl(url));
        // if fails, onClientDisconnected will schedule next
    });

    clientSockets.append(client);
    QString url = QString("ws://%1:%2").arg(host).arg(port);
    Logger::getInstance().log(QString("Connecting to %1").arg(url));
    client->open(QUrl(url));
}

void SocketClient::disconnect()
{
    for (QWebSocket *c : clientSockets) {
        if (c->state() == QAbstractSocket::ConnectedState) c->close();
        cleanupClientResources(c);
        c->deleteLater();
    }
    clientSockets.clear();
}

bool SocketClient::isConnected() const
{
    for (QWebSocket *c : clientSockets) if (c->state() == QAbstractSocket::ConnectedState) return true;
    return false;
}

int SocketClient::getClientCount() const
{
    return clientSockets.size();
}

QStringList SocketClient::getClientAddresses() const
{
    QStringList list;
    for (QWebSocket *c : clientSockets) list << getClientAddress(c);
    return list;
}

void SocketClient::sendMessageToServer(int index, const QString &message)
{
    if (index < 0 || index >= clientSockets.size()) {
        Logger::getInstance().error(QString("Invalid client index: %1").arg(index));
        return;
    }
    QWebSocket *c = clientSockets[index];
    if (c->state() != QAbstractSocket::ConnectedState) {
        Logger::getInstance().error(QString("Client %1 not connected").arg(getClientAddress(c)));
        return;
    }
    c->sendTextMessage(message);
    Logger::getInstance().log(QString("Sent message to %1: %2").arg(getClientAddress(c)).arg(message));
}

void SocketClient::sendMessageToServerByAddress(const QString &address, const QString &message)
{
    for (QWebSocket *c : clientSockets) {
        if (getClientAddress(c) == address) {
            if (c->state() == QAbstractSocket::ConnectedState) {
                c->sendTextMessage(message);
                Logger::getInstance().log(QString("Sent message to %1: %2").arg(address).arg(message));
                return;
            }
        }
    }
    Logger::getInstance().error(QString("No client found with address %1").arg(address));
}

// -------------------- Server slots --------------------

void SocketClient::onNewConnection()
{
    QWebSocket *client = webSocketServer->nextPendingConnection();
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("New client connected: %1").arg(addr));
    connect(client, &QWebSocket::textMessageReceived, this, &SocketClient::onServerClientTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &SocketClient::onServerClientDisconnected);
    serverClients.append(client);
    emit clientConnected(addr);
}

void SocketClient::onServerClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("Client disconnected: %1").arg(addr));
    serverClients.removeAll(client);
    emit clientDisconnected(addr);
    client->deleteLater();
}

void SocketClient::onServerClientTextMessageReceived(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("Message from client %1: %2").arg(addr).arg(message));
    emit messageReceived(message, addr);
}

// -------------------- Client slots --------------------

void SocketClient::onClientConnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("Connected to server: %1").arg(addr));
    // reset reconnect attempts/missed pongs and start heartbeat
    reconnectAttempts[client] = 0;
    missedPongs[client] = 0;
    QTimer *hb = heartbeatTimers.value(client, nullptr);
    if (hb) hb->start();
    // stop any pending reconnect timer
    QTimer *rt = reconnectTimers.value(client, nullptr);
    if (rt && rt->isActive()) rt->stop();

    emit connected();
    emit connectedToServer(addr);
}

void SocketClient::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("Disconnected from server: %1").arg(addr));
    // 停止发送心跳包
    QTimer *hb = heartbeatTimers.value(client, nullptr);
    if (hb && hb->isActive()) hb->stop();

    // 开始重连尝试
    int attempts = reconnectAttempts.value(client, 0);
    if (maxReconnectAttempts > 0 && attempts >= maxReconnectAttempts) {
        Logger::getInstance().error(QString("Max reconnect attempts reached for %1, cleaning up").arg(addr));
        // 清除资源
        cleanupClientResources(client);
        clientSockets.removeAll(client);
        client->deleteLater();
        emit errorOccurred(QString("Reconnect failed for %1 after %2 attempts").arg(addr).arg(attempts));
        return;
    }

    int shift = qMin(attempts, 6);
    int base = 1000;
    int cap = 30000;
    int delay = qMin(cap, base * (1 << shift));
    Logger::getInstance().log(QString("Scheduling reconnect to %1 in %2 ms (attempt %3)").arg(addr).arg(delay).arg(attempts+1));
    QTimer *rt = reconnectTimers.value(client, nullptr);
    if (rt) rt->start(delay);

    emit disconnected();
    emit disconnectedFromServer(addr);
}

void SocketClient::onClientTextMessageReceived(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("Message from server %1: %2").arg(addr).arg(message));
    // reset missed pongs on any incoming message as well (optional)
    missedPongs[client] = 0;
    emit messageReceived(message, addr);
}

void SocketClient::onClientError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    QString err = client ? client->errorString() : QString("unknown");
    Logger::getInstance().error(QString("Client error: %1").arg(err));
    emit errorOccurred(err);
}

QString SocketClient::getClientAddress(QWebSocket *client) const
{
    if (!client) return QString();
    return QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
}

// Cleanup timers and maps associated with a client
void SocketClient::cleanupClientResources(QWebSocket* client)
{
    if (!client) return;
    if (heartbeatTimers.contains(client)) {
        QTimer* t = heartbeatTimers.value(client);
        t->stop();
        t->deleteLater();
        heartbeatTimers.remove(client);
    }
    if (reconnectTimers.contains(client)) {
        QTimer* t = reconnectTimers.value(client);
        t->stop();
        t->deleteLater();
        reconnectTimers.remove(client);
    }
    clientHost.remove(client);
    clientPort.remove(client);
    reconnectAttempts.remove(client);
    missedPongs.remove(client);
}

// -------------------- Client pong handler --------------------
void SocketClient::onClientPong(quint64 elapsedTime)
{
    Q_UNUSED(elapsedTime);
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    missedPongs[client] = 0;
}

