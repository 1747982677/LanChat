#include "chat_service.h"
#include "utils/logger.h" // ��������־����
#include <QJsonDocument>
#include <QJsonObject>

ChatService::ChatService() : QObject(nullptr)
{
    m_socketClient = new SocketClient(this);

    // ���ӵײ� socket �ͻ��˵��źŵ�������Ĳ�
    connect(m_socketClient, &SocketClient::messageReceived, this, &ChatService::onSocketMessageReceived);
    connect(m_socketClient, &SocketClient::errorOccurred, this, &ChatService::onSocketError);
    connect(m_socketClient, &SocketClient::onlineAddressesReceived, this, &ChatService::onOnlineAddressesReceived);
    connect(m_socketClient, &SocketClient::connectedToServer, this, &ChatService::onConnectedToServer);

    // ��ʱ����ˢ�������û�
    connect(&m_onlineRefreshTimer, &QTimer::timeout, this, &ChatService::onRefreshOnlineUsers);
}

void ChatService::Init(quint16 serverPort)
{
    // 1. ���� WebSocket ������
    if (!m_socketClient->startServer(serverPort)) {
        Logger::getInstance().error("ChatService: Failed to start server.");
        emit errorOccurred("Failed to start server.");
        return;
    }
    Logger::getInstance().log(QString("ChatService: Server started on port %1").arg(m_socketClient->getServerPort()));

    // �״ι㲥
    m_socketClient->broadcastGetOnlineAddresses();

    // ������ʱˢ��
    if (m_refreshIntervalMs > 0) {
        m_onlineRefreshTimer.start(m_refreshIntervalMs);
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

void ChatService::sendMessage(const QString& content, const QString& receiverId)
{
    QString receiverAddress = receiverId;
    if (receiverAddress.isEmpty()) {
        emit errorOccurred("Receiver address is empty.");
        return;
    }

    // ����������ӣ���������ȥ��(client)�ͱ�����������(server)
    QStringList connectedClientAddrs = m_socketClient->getClientAddresses();
    QStringList connectedServerClientAddrs = m_socketClient->getServerClientAddresses();

    bool isAlreadyConnectedAsClient = connectedClientAddrs.contains(receiverAddress);
    bool isAlreadyConnectedAsServer = connectedServerClientAddrs.contains(receiverAddress);

    if (isAlreadyConnectedAsClient || isAlreadyConnectedAsServer) {
        // �Ѿ����ӣ�ֱ�ӷ����ı�
        if (isAlreadyConnectedAsClient) {
            m_socketClient->sendMessageToServerByAddress(receiverAddress, content);
        } else {
            m_socketClient->sendMessageToClientByAddress(receiverAddress, content);
        }
        Logger::getInstance().log(QString("Sent message directly to %1").arg(receiverAddress));
        emit messageSent(content, receiverAddress);
    }
    else if (m_onlineUsers.contains(receiverAddress)) {
        // ���ߵ�δ���ӣ��������ٷ���
        Logger::getInstance().log(QString("User %1 is online but not connected. Queuing message and connecting...").arg(receiverAddress));
        
        // ����Ϣ��������Ͷ���
        m_pendingMessages[receiverAddress].append(content);

        // ������ǵ�һ����������Ϣ����������
        if (m_pendingMessages[receiverAddress].size() == 1) {
            QStringList parts = receiverAddress.split(':');
            if (parts.size() == 2) {
                QString host = parts[0];
                quint16 port = parts[1].toUShort();
                m_socketClient->connectToHost(host, port);
            }
            else {
                Logger::getInstance().error(QString("Invalid receiver address format: %1").arg(receiverAddress));
                emit errorOccurred(QString("Invalid receiver address format: %1").arg(receiverAddress));
                m_pendingMessages.remove(receiverAddress);
            }
        }
    }
    else {
        // �û�������
        Logger::getInstance().error(QString("Cannot send message: User %1 is not online.").arg(receiverAddress));
        emit errorOccurred(QString("User %1 is not online.").arg(receiverAddress));
    }
}

QStringList ChatService::getOnlineUsers() const
{
    return m_onlineUsers;
}

void ChatService::onSocketMessageReceived(const QString& message, const QString& from)
{
    // ֱ����Ϊ�ı�ת��
    emit messageReceived(message, from);
}

void ChatService::onSocketError(const QString& error)
{
    // ֱ��ת���ײ�Ĵ���
    emit errorOccurred(error);
}

void ChatService::onOnlineAddressesReceived(const QStringList& addresses)
{
    m_onlineUsers = addresses;
    Logger::getInstance().log(QString("Online users updated: %1 found.").arg(m_onlineUsers.size()));
    emit onlineUsersUpdated(m_onlineUsers);
}

void ChatService::onConnectedToServer(const QString& address)
{
    // �ɹ����ӵ�һ���������󣬼���Ƿ��д����͸�������Ϣ
    if (m_pendingMessages.contains(address)) {
        Logger::getInstance().log(QString("Connection to %1 established. Sending queued messages...").arg(address));
        QStringList messagesToSend = m_pendingMessages.take(address); // ȡ�����Ƴ�
        for (const auto& text : messagesToSend) {
            m_socketClient->sendMessageToServerByAddress(address, text);
            emit messageSent(text, address);
        }
    }
}

void ChatService::onRefreshOnlineUsers()
{
    if (!m_socketClient) return;
    Logger::getInstance().log("Broadcasting to refresh online users...");
    m_socketClient->broadcastGetOnlineAddresses();
}
