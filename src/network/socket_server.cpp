#include "socket_server.h"
#include "utils/logger.h"
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SocketServer::SocketServer(QObject *parent)
    : QObject(parent), m_server(nullptr)
{
}

SocketServer::~SocketServer()
{
    stopServer();
}

bool SocketServer::startServer(quint16 port)
{
    if (m_server) {
        Logger::getInstance().log("WebSocket server already running");
        return false;
    }

    m_server = new QWebSocketServer(QStringLiteral("LanChat Central Server"),
                                    QWebSocketServer::NonSecureMode,
                                    this);
    
    if (!m_server->listen(QHostAddress::Any, port)) {
        QString err = m_server->errorString();
        Logger::getInstance().error(QString("WebSocket server start failed: %1").arg(err));
        emit serverError(err);
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QWebSocketServer::newConnection, this, &SocketServer::onNewConnection);
    
    const QHostAddress addr = m_server->serverAddress();
    const quint16 sport = m_server->serverPort();
    Logger::getInstance().log(QString("Central WebSocket server started on %1:%2").arg(addr.toString()).arg(sport));
    emit serverStarted(sport);
    return true;
}

void SocketServer::stopServer()
{
    if (!m_server) return;
    
    for (QWebSocket *client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    m_clientAddressToUserId.clear();
    m_userIdToClientAddress.clear();
    
    m_server->close();
    m_server->deleteLater();
    m_server = nullptr;
    
    Logger::getInstance().log("Central WebSocket server stopped");
    emit serverStopped();
}

bool SocketServer::isRunning() const
{
    return m_server && m_server->isListening();
}

quint16 SocketServer::getServerPort() const
{
    return (m_server && m_server->isListening()) ? m_server->serverPort() : 0;
}

bool SocketServer::sendMessageToUser(const QString& userId, const QString& message)
{
    if (!isRunning()) {
        Logger::getInstance().error("Cannot send: server not running");
        return false;
    }

    QWebSocket* client = getClientByUserId(userId);
    if (!client) {
        Logger::getInstance().error(QString("User %1 not found or not connected").arg(userId));
        return false;
    }

    if (client->state() != QAbstractSocket::ConnectedState) {
        Logger::getInstance().error(QString("User %1 not in connected state").arg(userId));
        return false;
    }

    client->sendTextMessage(message);
    Logger::getInstance().log(QString("Forwarded message to user %1: %2").arg(userId).arg(message));
    return true;
}

void SocketServer::sendMessageToClient(const QString& clientAddress, const QString& message)
{
    if (!isRunning()) {
        Logger::getInstance().error("Cannot send: server not running");
        return;
    }

    QWebSocket* client = getClientByAddress(clientAddress);
    if (!client) {
        Logger::getInstance().error(QString("Client %1 not found").arg(clientAddress));
        return;
    }

    if (client->state() != QAbstractSocket::ConnectedState) {
        Logger::getInstance().error(QString("Client %1 not connected").arg(clientAddress));
        return;
    }

    client->sendTextMessage(message);
    Logger::getInstance().log(QString("Sent message to client %1: %2").arg(clientAddress).arg(message));
}

void SocketServer::broadcastMessage(const QString& message)
{
    if (!isRunning()) {
        Logger::getInstance().error("Cannot broadcast: server not running");
        return;
    }
    
    int sentCount = 0;
    for (QWebSocket* client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
            sentCount++;
        }
    }
    
    Logger::getInstance().log(QString("Broadcasted message to %1 clients: %2").arg(sentCount).arg(message));
}

int SocketServer::getClientCount() const
{
    return m_clients.size();
}

QStringList SocketServer::getClientAddresses() const
{
    QStringList list;
    for (QWebSocket *client : m_clients) {
        list << getClientAddress(client);
    }
    return list;
}

QStringList SocketServer::getOnlineUserIds() const
{
    return m_userIdToClientAddress.keys();
}

void SocketServer::registerClientUserId(const QString& clientAddress, const QString& userId)
{
    if (userId.isEmpty() || clientAddress.isEmpty()) {
        Logger::getInstance().error("Cannot register: empty userId or address");
        return;
    }

    // 移除旧的映射（如果存在）
    QString oldUserId = m_clientAddressToUserId.value(clientAddress);
    if (!oldUserId.isEmpty() && oldUserId != userId) {
        m_userIdToClientAddress.remove(oldUserId);
    }

    // 建立新映射
    m_clientAddressToUserId[clientAddress] = userId;
    m_userIdToClientAddress[userId] = clientAddress;
    
    Logger::getInstance().log(QString("Registered client %1 with userId %2").arg(clientAddress).arg(userId));
}

void SocketServer::disconnectClient(const QString& clientAddress)
{
    QWebSocket* client = getClientByAddress(clientAddress);
    if (client) {
        client->close();
        Logger::getInstance().log(QString("Disconnecting client: %1").arg(clientAddress));
    } else {
        Logger::getInstance().error(QString("Client not found: %1").arg(clientAddress));
    }
}

void SocketServer::onNewConnection()
{
    QWebSocket *client = m_server->nextPendingConnection();
    if (!client) return;
    
    QString addr = getClientAddress(client);
    Logger::getInstance().log(QString("New client connected: %1").arg(addr));
    
    connect(client, &QWebSocket::textMessageReceived, this, &SocketServer::onClientTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &SocketServer::onClientDisconnected);
    
    m_clients.append(client);
    emit clientConnected(addr);
}

void SocketServer::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    QString addr = getClientAddress(client);
    QString userId = m_clientAddressToUserId.value(addr);
    
    Logger::getInstance().log(QString("Client disconnected: %1 (userId: %2)").arg(addr).arg(userId));
    
    // 清理映射
    m_clientAddressToUserId.remove(addr);
    if (!userId.isEmpty()) {
        m_userIdToClientAddress.remove(userId);
    }
    
    m_clients.removeAll(client);
    emit clientDisconnected(addr, userId);
    client->deleteLater();
}

void SocketServer::onClientTextMessageReceived(const QString& message)
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    QString fromAddr = getClientAddress(client);
    QString fromUserId = m_clientAddressToUserId.value(fromAddr);
    
    Logger::getInstance().log(QString("Message from client %1 (userId: %2): %3")
        .arg(fromAddr).arg(fromUserId).arg(message));
    
    // 尝试解析消息为 JSON
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);
    
    if (!doc.isNull() && err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        
        // 检查是否是注册消息
        QString msgType = obj.value("type").toString();
        if (msgType == "register") {
            QString userId = obj.value("userId").toString();
            if (!userId.isEmpty()) {
                registerClientUserId(fromAddr, userId);
                
                // 发送注册成功响应
                QJsonObject response;
                response["type"] = "register_ack";
                response["success"] = true;
                response["userId"] = userId;
                client->sendTextMessage(QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact)));
                return;
            }
        }
        
        // 处理查询在线用户请求
        if (msgType == "query_online_users") {
            QJsonObject response;
            response["type"] = "online_users";
            QJsonArray usersArray;
            
            // 获取所有在线用户ID
            for (const QString& userId : m_userIdToClientAddress.keys()) {
                usersArray.append(userId);
            }
            
            response["users"] = usersArray;
            client->sendTextMessage(QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact)));
            Logger::getInstance().log(QString("Sent online users list to %1: %2 users").arg(fromUserId).arg(usersArray.size()));
            return;
        }
        
        // 检查是否需要转发
        QString receiverId = obj.value("receiverId").toString();
        if (!receiverId.isEmpty()) {
            QString messageId = obj.value("messageId").toString();
            
            // 转发消息给目标用户
            if (sendMessageToUser(receiverId, message)) {
                Logger::getInstance().log(QString("Message forwarded from %1 to %2").arg(fromUserId).arg(receiverId));
                emit messageForwarded(messageId, receiverId);
                
                // 发送转发成功回执给发送者
                QJsonObject ack;
                ack["type"] = "message_ack";
                ack["msgId"] = messageId;
                ack["success"] = true;
                client->sendTextMessage(QString::fromUtf8(QJsonDocument(ack).toJson(QJsonDocument::Compact)));
            } else {
                Logger::getInstance().error(QString("Failed to forward message from %1 to %2: user not online")
                    .arg(fromUserId).arg(receiverId));
                emit messageForwardFailed(messageId, receiverId, "User not online");
                
                // 发送转发失败回执给发送者
                QJsonObject ack;
                ack["type"] = "message_ack";
                ack["msgId"] = messageId;
                ack["success"] = false;
                ack["reason"] = "User not online";
                client->sendTextMessage(QString::fromUtf8(QJsonDocument(ack).toJson(QJsonDocument::Compact)));
            }
            return;
        }
    }
    
    // 普通消息，发送给上层处理
    emit messageReceived(message, fromAddr, fromUserId);
}

QString SocketServer::getClientAddress(QWebSocket *client) const
{
    if (!client) return QString();
    return QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
}

QWebSocket* SocketServer::getClientByAddress(const QString& address) const
{
    for (QWebSocket* client : m_clients) {
        if (getClientAddress(client) == address) {
            return client;
        }
    }
    return nullptr;
}

QWebSocket* SocketServer::getClientByUserId(const QString& userId) const
{
    QString address = m_userIdToClientAddress.value(userId);
    if (address.isEmpty()) {
        return nullptr;
    }
    return getClientByAddress(address);
}
