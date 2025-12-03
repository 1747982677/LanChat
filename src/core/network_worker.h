#ifndef NETWORK_WORKER_H
#define NETWORK_WORKER_H

#include "core/base_worker.h"
#include "network/socket_client.h"
#include <QJsonObject>

/**
 * @brief ����ͨ�� Worker
 * 
 * �����ڶ����������߳��У�����
 * 1. ��Ϣ�շ�
 * 2. ���ӹ���
 * 3. ����ά��
 * 4. ����״̬���
 */
class NetworkWorker : public BaseWorker
{
    Q_OBJECT

public:
    explicit NetworkWorker(QObject* parent = nullptr);
    ~NetworkWorker() override;

    bool initialize() override;
    void cleanup() override;

public slots:
    /**
     * @brief ���ӵ�������
     * @param host ��������ַ
     * @param port �������˿�
     */
    void connectToServer(const QString& host, quint16 port);

    /**
     * @brief �Ͽ�����
     */
    void disconnectFromServer();

    /**
     * @brief ���� JSON ��Ϣ
     * @param message JSON ����
     */
    void sendMessage(const QJsonObject& message);

    /**
     * @brief �����ı���Ϣ
     * @param text �ı�����
     */
    void sendTextMessage(const QString& text);

    /**
     * @brief ����������ģʽ
     * @param port �����˿�
     */
    void startServer(quint16 port);

    /**
     * @brief ֹͣ������ģʽ
     */
    void stopServer();

signals:
    /**
     * @brief ���ӳɹ��ź�
     */
    void connected();

    /**
     * @brief �Ͽ������ź�
     */
    void disconnected();

    /**
     * @brief �յ���Ϣ�ź�
     * @param message JSON ��Ϣ����
     * @param from �����ߵ�ַ
     */
    void messageReceived(const QJsonObject& message, const QString& from);

    /**
     * @brief �յ��ı���Ϣ�ź�
     */
    void textMessageReceived(const QString& text, const QString& from);

    /**
     * @brief ����״̬�仯�ź�
     */
    void connectionStateChanged(bool isConnected);


    // ��Ϣ���ͽ���ź�
    void messageSendSuccess(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& reason);


private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketMessageReceived(const QString& message, const QString& from);
    void onSocketError(const QString& error);

private:
    SocketClient* m_socketClient;
    bool m_isConnected;
};

#endif // NETWORK_WORKER_H
