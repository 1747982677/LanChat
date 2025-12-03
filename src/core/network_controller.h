#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "core/base_controller.h"
#include <QJsonObject>

/**
 * @brief ����ͨ�ſ�����
 * 
 * ���������̣߳�����
 * 1. ���� NetworkWorker ����������
 * 2. �ṩ����������ⲿ�ӿ�
 * 3. ת�������¼����ϲ�
 */
class NetworkController : public BaseController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject* parent = nullptr);
    ~NetworkController() override;

    bool initialize() override;

    /**
     * @brief ��ȡ����
     */
    static NetworkController& instance();

public slots:
    /**
     * @brief ���ӵ�������
     */
    void connectToServer(const QString& host, quint16 port);

    /**
     * @brief �Ͽ�����
     */
    void disconnectFromServer();

    /**
     * @brief ������Ϣ
     */
    void sendMessage(const QJsonObject& message);

    /**
     * @brief �����ı���Ϣ
     */
    void sendTextMessage(const QString& text);

    /**
     * @brief ����������
     */
    void startServer(quint16 port);

    /**
     * @brief ֹͣ������
     */
    void stopServer();

signals:
    // ���͸� Worker ���ź�
    void requestConnect(const QString& host, quint16 port);
    void requestDisconnect();
    void requestSendMessage(const QJsonObject& message);
    void requestSendTextMessage(const QString& text);
    void requestStartServer(quint16 port);
    void requestStopServer();

    // �� Worker ���յ��źţ�ת����
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& message, const QString& from);
    void textMessageReceived(const QString& text, const QString& from);
    void connectionStateChanged(bool isConnected);

    // ��Ϣ���ͽ���ź�
    void messageSendSuccess(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& reason);

protected:
    QObject* createWorker() override;

private:
    void connectSignals();

    static NetworkController* s_instance;
};

#endif // NETWORK_CONTROLLER_H
