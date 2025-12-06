#include "chat_service.h"
#include "utils/logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QEventLoop>

ChatService::ChatService() : QObject(nullptr), m_isServerMode(false), m_serverPort(0), m_isInitialized(false)
{
    m_socketClient = new SocketClient(this);

    // 连接客户端信号
    connect(m_socketClient, &SocketClient::connected, this, [this]() {
        Logger::getInstance().log("Connected to central server");
        // 连接成功后，立即注册自己的 userId
        if (!m_myUserId.isEmpty()) {
            registerToServer();
        }
    });

    connect(m_socketClient, &SocketClient::disconnected, this, [this]() {
        Logger::getInstance().log("Disconnected from central server");
    });

    connect(m_socketClient, &SocketClient::messageReceived, this, &ChatService::onSocketMessageReceived);
    
    connect(m_socketClient, &SocketClient::errorOccurred, this, [this](const QString& error) {
        Logger::getInstance().error(QString("Client error: %1").arg(error));
        emit errorOccurred(error);
    });

    // 定时器：刷新在线用户（可选）
    connect(&m_onlineRefreshTimer, &QTimer::timeout, this, &ChatService::onRefreshOnlineUsers);
    
    // UDP 发现套接字
    m_discoverySocket = new QUdpSocket(this);
    bool ok = m_discoverySocket->bind(QHostAddress::AnyIPv4, m_discoveryPort, 
                            QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!ok) {
        Logger::getInstance().error("UDP bind failed: " + m_discoverySocket->errorString());
    }
    connect(m_discoverySocket, &QUdpSocket::readyRead, this, &ChatService::onDiscoveryResponse);
    
    // 发现超时定时器
    m_discoveryTimer = new QTimer(this);
    m_discoveryTimer->setSingleShot(true);
    connect(m_discoveryTimer, &QTimer::timeout, this, &ChatService::onDiscoveryTimeout);
}

bool ChatService::initialize(const QString& myUserId)
{
    if (m_isInitialized) {
        Logger::getInstance().log("ChatService already initialized");
        return true;
    }
    
    if (myUserId.isEmpty()) {
        Logger::getInstance().error("Cannot initialize: myUserId is empty");
        return false;
    }
    
    m_myUserId = myUserId;
    m_isInitialized = true;
    
    Logger::getInstance().log(QString("ChatService initialized for user %1").arg(m_myUserId));
    return true;
}

bool ChatService::autoInit(quint16 serverPort, int timeoutMs)
{
    if (!m_isInitialized) {
        Logger::getInstance().error("Must call initialize() before autoInit()");
        return false;
    }
    
    Logger::getInstance().log(QString("Auto-detecting server on port %1...").arg(serverPort));
    
    // 发送 UDP 广播检测服务器
    m_serverFound = false;
    m_discoveredServerAddress.clear();
    
    // 构造发现请求包
    QJsonObject discoveryRequest;
    discoveryRequest["type"] = "discover_request";
    discoveryRequest["port"] = static_cast<int>(serverPort);
    
    QByteArray data = QJsonDocument(discoveryRequest).toJson(QJsonDocument::Compact);
    
    // 发送到广播地址和本地回环
    /*m_discoverySocket->writeDatagram(data, QHostAddress::Broadcast, m_discoveryPort);
    m_discoverySocket->writeDatagram(data, QHostAddress::LocalHost, m_discoveryPort);*/
    for (auto& iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            iface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        for (auto& entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            QHostAddress broadcast = entry.broadcast();
            if (broadcast.isNull()) continue;

            m_discoverySocket->writeDatagram(data, broadcast, m_discoveryPort);
        }
    }
    
    Logger::getInstance().log(QString("Sent server discovery broadcast (timeout: %1ms)").arg(timeoutMs));
    
    // 启动超时定时器
    m_discoveryTimer->start(timeoutMs);
    
    // 使用事件循环等待响应或超时
    QEventLoop loop;
    connect(m_discoveryTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &ChatService::serverDiscovered, &loop, &QEventLoop::quit);
    loop.exec();
    
    // 根据检测结果初始化
    if (m_serverFound && !m_discoveredServerAddress.isEmpty()) {
        Logger::getInstance().log(QString("Server found at %1, connecting as client").arg(m_discoveredServerAddress));
        
        QStringList parts = m_discoveredServerAddress.split(':');
        if (parts.size() == 2) {
            return initAsClient(parts[0], parts[1].toUShort());
        } else {
            return initAsClient(m_discoveredServerAddress, serverPort);
        }
    } else {
        Logger::getInstance().log("No server found, starting as server");
        return initAsServer(serverPort);
    }
}

bool ChatService::initAsServer(quint16 serverPort)
{
    if (!m_isInitialized) {
        Logger::getInstance().error("Must call initialize() before initAsServer()");
        return false;
    }
    
    m_serverHost = "127.0.0.1"; // 服务器模式下连接到本地
    m_serverPort = serverPort;
    m_isServerMode = true;
    
    // 1. 启动 WebSocket 服务器
    m_socketServer = new SocketServer(this);
    if (!m_socketServer->startServer(serverPort)) {
        Logger::getInstance().error("ChatService: Failed to start server.");
        emit errorOccurred("Failed to start server.");
        return false;
    }
    
    Logger::getInstance().log(QString("ChatService: Server started on port %1 for user %2")
        .arg(serverPort).arg(m_myUserId));
    
    // 启动 UDP 响应服务（响应其他客户端的发现请求）
    disconnect(m_discoverySocket, &QUdpSocket::readyRead, this, &ChatService::onDiscoveryResponse);
    connect(m_discoverySocket, &QUdpSocket::readyRead, this, [this, serverPort]() {
        while (m_discoverySocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(int(m_discoverySocket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;
            m_discoverySocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            
            QJsonDocument doc = QJsonDocument::fromJson(datagram);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.value("type").toString() == "discover_request") {
                    quint16 requestedPort = static_cast<quint16>(obj.value("port").toInt());
                    if (requestedPort == serverPort) {
                        // 响应发现请求
                        QJsonObject response;
                        response["type"] = "discover_response";
                        response["port"] = static_cast<int>(serverPort);
                        response["userId"] = m_myUserId;
                        
                        // 获取本机 IP 地址
                        QString myAddress = "127.0.0.1";
                        const QHostAddress& localhost = QHostAddress(QHostAddress::LocalHost);
                        for (const QHostAddress& address : QNetworkInterface::allAddresses()) {
                            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                                // 优先选择局域网IP
                                if (address.toString().startsWith("192.168.")) {
                                    myAddress = address.toString();
                                    break; // 找到合适的，就跳出循环
                                }
                                // 否则，暂存一个非环回地址
                                myAddress = address.toString();
                            }
                        }
                        /*for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
                            if (iface.flags() & QNetworkInterface::IsUp && 
                                !(iface.flags() & QNetworkInterface::IsLoopBack)) {
                                for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                                        myAddress = entry.ip().toString();
                                        break;
                                    }
                                }
                            }
                        }*/
                    
                        response["address"] = myAddress;
                        
                        QByteArray responseData = QJsonDocument(response).toJson(QJsonDocument::Compact);
                        m_discoverySocket->writeDatagram(responseData, sender, senderPort);
                        m_discoverySocket->writeDatagram(responseData, QHostAddress::Broadcast, m_discoveryPort);
                        m_discoverySocket->writeDatagram(responseData, QHostAddress::LocalHost, m_discoveryPort);
                        //m_discoverySocket->writeDatagram(responseData, sender, m_discoveryPort);
                        m_discoverySocket->waitForReadyRead(100);
                        
                        Logger::getInstance().log(QString("Sent discovery response to %1:%2")
                            .arg(sender.toString()).arg(senderPort));
                        Logger::getInstance().log(QString("MyAddress %1")
                            .arg(myAddress));
                    }
                }
            }
        }
    });
    
    // 2. 作为客户端连接到自己的服务器
    m_socketClient->connectToServer(m_serverHost, m_serverPort);
    
    Logger::getInstance().log(QString("ChatService: Connecting to own server at %1:%2")
        .arg(m_serverHost).arg(m_serverPort));
    
    // 启动定时刷新（如果需要）
    if (m_refreshIntervalMs > 0) {
        m_onlineRefreshTimer.start(m_refreshIntervalMs);
    }
    
    return true;
}

bool ChatService::initAsClient(const QString& serverHost, quint16 serverPort)
{
    if (!m_isInitialized) {
        Logger::getInstance().error("Must call initialize() before initAsClient()");
        return false;
    }
    
    m_serverHost = serverHost;
    m_serverPort = serverPort;
    m_isServerMode = false;
    
    // 只作为客户端连接到服务器
    m_socketClient->connectToServer(serverHost, serverPort);
    
    Logger::getInstance().log(QString("ChatService: Client mode, connecting to server %1:%2 with user %3")
        .arg(serverHost).arg(serverPort).arg(m_myUserId));
    
    // 启动定时刷新（如果需要）
    if (m_refreshIntervalMs > 0) {
        m_onlineRefreshTimer.start(m_refreshIntervalMs);
    }
    
    return true;
}

void ChatService::Init(quint16 port, const QString& myUserId)
{
    // 先初始化用户ID
    initialize(myUserId);
    
    // 兼容旧接口：如果端口是8080，作为服务器；否则作为客户端连接到8080
    if (port == 8080) {
        initAsServer(8080);
    } else {
        initAsClient("127.0.0.1", 8080);
    }
}

void ChatService::setOnlineRefreshInterval(int intervalMs)
{
    m_refreshIntervalMs = intervalMs;
    if (intervalMs <= 0) {
        m_onlineRefreshTimer.stop();
        Logger::getInstance().log("Online user auto-refresh disabled.");
    } else {
        m_onlineRefreshTimer.start(intervalMs);
        Logger::getInstance().log(QString("Online user auto-refresh interval set to %1 ms").arg(intervalMs));
    }
}

void ChatService::sendMessage(const LanChat::Message& message)
{
    if (!m_socketClient->isConnected()) {
        Logger::getInstance().error("Cannot send message: not connected to server");
        emit errorOccurred("Not connected to server");
        return;
    }

    // 序列化消息为 JSON
    QJsonObject jsonObj = message.toJson();
    QString payload = QString::fromUtf8(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));

    // 直接发送给服务器，服务器会根据 receiverId 转发
    m_socketClient->sendMessage(payload);
    
    Logger::getInstance().log(QString("Sent message to server for user %1: %2")
        .arg(message.receiverId).arg(message.content));
    
    // 注意：这里先触发 messageSent，实际应该等待服务器 ACK
    // 在生产环境中，应该等待 message_ack 再触发
    emit messageSent(message);
}

QStringList ChatService::getOnlineUsers() const
{
    return m_onlineUserIds;
}

void ChatService::registerUser(const QString& userId, const QString& address)
{
    // 在客户端-服务器模式下，此方法用于本地缓存在线用户
    if (!userId.isEmpty()) {
        if (!m_onlineUserIds.contains(userId)) {
            m_onlineUserIds.append(userId);
            Logger::getInstance().log(QString("Registered online user %1").arg(userId));
        }
    }
}

QString ChatService::getUserAddress(const QString& userId) const
{
    Q_UNUSED(userId);
    return QString();
}

QString ChatService::getAddressUserId(const QString& address) const
{
    Q_UNUSED(address);
    return QString();
}

QString ChatService::generateTempUserId(const QString& address)
{
    Q_UNUSED(address);
    return QString();
}

void ChatService::registerToServer()
{
    if (!m_socketClient->isConnected()) {
        Logger::getInstance().error("Cannot register: not connected to server");
        return;
    }

    // 发送注册消息给服务器
    QJsonObject registerMsg;
    registerMsg["type"] = "register";
    registerMsg["userId"] = m_myUserId;
    
    QString payload = QString::fromUtf8(QJsonDocument(registerMsg).toJson(QJsonDocument::Compact));
    m_socketClient->sendMessage(payload);
    
    Logger::getInstance().log(QString("Sent registration to server for user %1").arg(m_myUserId));
}

void ChatService::requestOnlineUsers()
{
    if (!m_socketClient->isConnected()) {
        Logger::getInstance().error("Cannot request online users: not connected to server");
        return;
    }

    // 发送查询在线用户的请求
    QJsonObject queryMsg;
    queryMsg["type"] = "query_online_users";
    
    QString payload = QString::fromUtf8(QJsonDocument(queryMsg).toJson(QJsonDocument::Compact));
    m_socketClient->sendMessage(payload);
    
    Logger::getInstance().log("Requested online users from server");
}

void ChatService::onSocketMessageReceived(const QString& message)
{
    // 解析消息
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        Logger::getInstance().error(QString("Received invalid JSON: %1").arg(message));
        return;
    }

    if (!doc.isObject()) {
        Logger::getInstance().error(QString("Received non-object JSON: %1").arg(message));
        return;
    }

    QJsonObject obj = doc.object();
    QString msgType = obj.value("type").toString();

    // 处理注册确认
    if (msgType == "register_ack") {
        bool success = obj.value("success").toBool();
        if (success) {
            Logger::getInstance().log("Registration successful");
            // 注册成功后，可以请求在线用户列表
            requestOnlineUsers();
        } else {
            Logger::getInstance().error("Registration failed");
            emit errorOccurred("Registration failed");
        }
        return;
    }

    // 处理消息发送确认
    if (msgType == "message_ack") {
        QString msgId = obj.value("msgId").toString();
        bool success = obj.value("success").toBool();
        if (success) {
            Logger::getInstance().log(QString("Message %1 sent successfully").arg(msgId));
        } else {
            QString reason = obj.value("reason").toString();
            Logger::getInstance().error(QString("Message %1 failed: %2").arg(msgId).arg(reason));
            emit errorOccurred(QString("Message send failed: %1").arg(reason));
        }
        return;
    }

    // 处理在线用户列表
    if (msgType == "online_users") {
        m_onlineUserIds.clear();
        QJsonArray usersArray = obj.value("users").toArray();
        for (const QJsonValue& val : usersArray) {
            QString userId = val.toString();
            if (!userId.isEmpty() && userId != m_myUserId) {
                m_onlineUserIds.append(userId);
            }
        }
        Logger::getInstance().log(QString("Received online users list: %1 users").arg(m_onlineUserIds.size()));
        emit onlineUsersUpdated(m_onlineUserIds);
        return;
    }

    // 处理接收到的聊天消息
    LanChat::Message msg = LanChat::Message::fromJson(obj);
    
    if (!msg.senderId.isEmpty()) {
        Logger::getInstance().log(QString("Received message from user %1: %2")
            .arg(msg.senderId).arg(msg.content));
        emit messageReceived(msg);
    } else {
        Logger::getInstance().error(QString("Received message with invalid format: %1").arg(message));
    }
}

void ChatService::onRefreshOnlineUsers()
{
    if (m_socketClient->isConnected()) {
        Logger::getInstance().log("Refreshing online users...");
        requestOnlineUsers();
    }
}

void ChatService::onDiscoveryTimeout()
{
    Logger::getInstance().log("Server discovery timeout - no server found");
    m_serverFound = false;
    emit serverDiscovered(false, QString());
}

void ChatService::onDiscoveryResponse()
{
    while (m_discoverySocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_discoverySocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        m_discoverySocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString rawDatagramStr = QString::fromUtf8(datagram);
        Logger::getInstance().log(QString("onDiscovered found received: %1").arg(rawDatagramStr));
        
        QJsonDocument doc = QJsonDocument::fromJson(datagram);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.value("type").toString() == "discover_response") {
                Logger::getInstance().log("is discover_response");
                QString address = obj.value("address").toString();
                quint16 port = static_cast<quint16>(obj.value("port").toInt());
                QString userId = obj.value("userId").toString();
                
                m_serverFound = true;
                m_discoveredServerAddress = QString("%1:%2").arg(address).arg(port);
                
                Logger::getInstance().log(QString("Discovered server at %1 (user: %2)")
                    .arg(m_discoveredServerAddress).arg(userId));
                
                // 停止超时定时器
                if (m_discoveryTimer->isActive()) {
                    m_discoveryTimer->stop();
                }
                
                emit serverDiscovered(true, m_discoveredServerAddress);
                return; // 只处理第一个响应
            }
        }
    }
}
