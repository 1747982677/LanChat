#ifndef INETWORK_H
#define INETWORK_H

#include <QObject>
#include <QString>

// 抽象网络接口，便于 ChatService 解耦与单元测试替换 Mock
class INetwork : public QObject {
    Q_OBJECT
public:
    explicit INetwork(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~INetwork() = default;

    // 发送消息 payload，同时携带本地 localId 以便回执关联
    virtual void sendMessage(const QString& payload, qint64 localId) = 0;

signals:
    // 当服务端确认发送结果时发出（network 层应带回 localId）
    void sendAck(qint64 localId, bool success);

    // 当收到远端消息时发出
    void messageReceived(const QString& json);
};

#endif // INETWORK_H