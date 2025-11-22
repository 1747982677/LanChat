#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class SocketClient : public QObject
{
    Q_OBJECT

public:
    explicit SocketClient(QObject *parent = nullptr);
    ~SocketClient();
    
    void connectToHost(const QString& host, quint16 port);
    void disconnect();
    void sendMessage(const QString& message);
    
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString& message);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* socket;
};

#endif // SOCKET_CLIENT_H
