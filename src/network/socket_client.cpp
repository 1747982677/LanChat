#include "socket_client.h"
#include "utils/logger.h"

#include <QHostAddress>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>

// -------------------- Construction --------------------
SocketClient::SocketClient(QObject *parent)
    : QObject(parent), webSocketServer(nullptr), maxReconnectAttempts(10)
{
    onlineQueryTimer = new QTimer(this);
    onlineQueryTimer->setSingleShot(true);
    connect(onlineQueryTimer, &QTimer::timeout, this, [this]() {
        // 超时后直接发射收集到的在线地址
        QStringList list = onlineQueryResponses.values();
        Logger::getInstance().log(QString("Collected %1 online addresses via UDP broadcast").arg(list.size()));
        emit onlineAddressesReceived(list);
        onlineQueryResponses.clear();
    });

    // UDP discovery socket - 所有实例监听同一个 discoveryPort
    discoverySocket = new QUdpSocket(this);
    // 开启地址复用,允许多个进程绑定同一端口
    // discoverySocket->setSocketOption(QAbstractSocket::ReuseAddressHint, 1);
    // 开启回环,确保本机广播能被接收
    discoverySocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, true);
    
    // 绑定到固定的 discoveryPort,使用 ShareAddress + ReuseAddressHint
    if (!discoverySocket->bind(QHostAddress::AnyIPv4, discoveryPort, 
                               QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        Logger::getInstance().error(QString("UDP discovery bind to port %1 failed: %2")
            .arg(discoveryPort).arg(discoverySocket->errorString()));
    } else {
        connect(discoverySocket, &QUdpSocket::readyRead, this, &SocketClient::onUdpReadyRead);
        Logger::getInstance().log(QString("UDP discovery socket successfully bound to port %1").arg(discoveryPort));
    }
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

    if (discoverySocket) {
        discoverySocket->close();
        discoverySocket->deleteLater();
        discoverySocket = nullptr;
    }
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
    // 打印服务 IP 与端口（注意 Any 绑定时为 0.0.0.0，实际本机地址可通过 network interfaces 获取）
    const QHostAddress addr = webSocketServer->serverAddress();
    const quint16 sport = webSocketServer->serverPort();
    Logger::getInstance().log(QString("WebSocket server started on %1:%2").arg(addr.toString()).arg(sport));
    emit serverStarted(sport);
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

    // 解析 JSON 控制消息（online_query / online_response）
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);
    if (!doc.isNull() && err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        QString type = obj.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("online_query")) {
            // 收到查询 -> 以在线响应回复（告诉对方本端在服务器视角的 peer 地址）
            QJsonObject resp;
            resp.insert(QStringLiteral("type"), QStringLiteral("online_response"));
            resp.insert(QStringLiteral("address"), QJsonValue(getClientAddress(client)));
            client->sendTextMessage(QString::fromUtf8(QJsonDocument(resp).toJson(QJsonDocument::Compact)));
            return; // 控制消息，不触发普通 messageReceived
        } else if (type == QStringLiteral("online_response")) {
            QString addr = obj.value(QStringLiteral("address")).toString();
            if (!addr.isEmpty()) {
                onlineQueryResponses.insert(addr);
            }
            return; // 控制消息，不触发普通 messageReceived
        }
    }

    // 非控制消息，保持原有行为
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

    // 解析 JSON 控制消息（online_query / online_response）
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);
    if (!doc.isNull() && err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        QString type = obj.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("online_query")) {
            // 收到查询 -> 作为客户端回复本端局部地址（localAddress:localPort）
            QJsonObject resp;
            resp.insert(QStringLiteral("type"), QStringLiteral("online_response"));
            QString myAddr = QString("%1:%2").arg(client->localAddress().toString()).arg(client->localPort());
            resp.insert(QStringLiteral("address"), QJsonValue(myAddr));
            client->sendTextMessage(QString::fromUtf8(QJsonDocument(resp).toJson(QJsonDocument::Compact)));
            return; // 控制消息，不触发普通 messageReceived
        } else if (type == QStringLiteral("online_response")) {
            QString addr = obj.value(QStringLiteral("address")).toString();
            if (!addr.isEmpty()) {
                onlineQueryResponses.insert(addr);
            }
            return; // 控制消息，不触发普通 messageReceived
        }
    }

    // 非控制消息，保持原有行为
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


void SocketClient::broadcastGetOnlineAddresses(int timeoutMs)
{
    onlineQueryResponses.clear();

    // 🔧 UDP 广播:在请求中直接携带本机 WebSocket 服务器的所有地址
    if (discoverySocket && isServerRunning()) {
        const quint16 wsPort = webSocketServer->serverPort();
        QJsonArray myAddresses;
        
        // 枚举本机所有 IPv4 地址(包括回环 127.0.0.1)
        for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
            if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                QHostAddress addr = entry.ip();
                if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                    myAddresses.append(QString("%1:%2").arg(addr.toString()).arg(wsPort));
                }
            }
        }
        
        if (!myAddresses.isEmpty()) {
            QJsonObject packet;
            packet.insert(QStringLiteral("type"), QStringLiteral("discover"));
            packet.insert(QStringLiteral("addresses"), myAddresses);
            
            QByteArray payload = QJsonDocument(packet).toJson(QJsonDocument::Compact);
            
            // 发送到标准广播地址(局域网)
            discoverySocket->writeDatagram(payload, QHostAddress::Broadcast, discoveryPort);
            
            // 发送到本机回环地址(同机多实例)
            discoverySocket->writeDatagram(payload, QHostAddress::LocalHost, discoveryPort);
            
            Logger::getInstance().log(QString("Sent UDP broadcast with %1 addresses (LAN + localhost)")
                .arg(myAddresses.size()));
        }
    }

    // WebSocket 已有连接查询(保持不变)
    if (isServerRunning() || !clientSockets.isEmpty()) {
        QJsonObject q;
        q.insert(QStringLiteral("type"), QStringLiteral("online_query"));
        QString payload = QString::fromUtf8(QJsonDocument(q).toJson(QJsonDocument::Compact));

        if (isServerRunning()) {
            for (QWebSocket *c : serverClients) {
                if (c->state() == QAbstractSocket::ConnectedState)
                    c->sendTextMessage(payload);
            }
        }
        for (QWebSocket *c : clientSockets) {
            if (c->state() == QAbstractSocket::ConnectedState)
                c->sendTextMessage(payload);
        }
    }

    if (timeoutMs <= 0) timeoutMs = 1000;
    onlineQueryTimer->start(timeoutMs);
}

void SocketClient::onUdpReadyRead()
{
    while (discoverySocket && discoverySocket->hasPendingDatagrams()) {
        QHostAddress sender;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(int(discoverySocket->pendingDatagramSize()));
        discoverySocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &err);
        if (!doc.isNull() && err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QString type = obj.value(QStringLiteral("type")).toString();
            
            // 🔧 简化:只处理 discover 类型,直接提取地址,不响应
            if (type == QStringLiteral("discover")) {
                QJsonArray addresses = obj.value(QStringLiteral("addresses")).toArray();
                if (!addresses.isEmpty()) {
                    for (const QJsonValue &v : addresses) {
                        if (v.isString()) {
                            QString addr = v.toString();
                            onlineQueryResponses.insert(addr);
                        }
                    }
                    Logger::getInstance().log(QString("Received UDP discover from %1:%2 with %3 addresses")
                        .arg(sender.toString()).arg(senderPort).arg(addresses.size()));
                }
            }
        }
    }
}

