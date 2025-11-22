#include "socket_client.h"
#include "utils/logger.h"

SocketClient::SocketClient(QObject *parent)
    : QObject(parent), socket(nullptr)
{
    socket = new QTcpSocket(this);
    
    connect(socket, &QTcpSocket::connected, this, &SocketClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &SocketClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &SocketClient::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &SocketClient::onError);
}

SocketClient::~SocketClient()
{
    if (socket) {
        socket->close();
    }
}

void SocketClient::connectToHost(const QString& host, quint16 port)
{
    Logger::getInstance().log(QString("Connecting to %1:%2").arg(host).arg(port));
    socket->connectToHost(host, port);
}

void SocketClient::disconnect()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

void SocketClient::sendMessage(const QString& message)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = message.toUtf8();
        socket->write(data);
        socket->flush();
        Logger::getInstance().log("Message sent: " + message);
    } else {
        Logger::getInstance().error("Cannot send message: not connected");
    }
}

bool SocketClient::isConnected() const
{
    return socket && socket->state() == QAbstractSocket::ConnectedState;
}

void SocketClient::onConnected()
{
    Logger::getInstance().log("Connected to server");
    emit connected();
}

void SocketClient::onDisconnected()
{
    Logger::getInstance().log("Disconnected from server");
    emit disconnected();
}

void SocketClient::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    Logger::getInstance().log("Message received: " + message);
    emit messageReceived(message);
}

void SocketClient::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString = socket->errorString();
    Logger::getInstance().error("Socket error: " + errorString);
    emit errorOccurred(errorString);
}
