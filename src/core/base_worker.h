#ifndef BASE_WORKER_H
#define BASE_WORKER_H

#include <QObject>

/**
 * @brief Worker ����
 * 
 * ���� Worker �Ļ��࣬�����ڶ����߳���
 * Worker ����ִ�о���ĺ�ʱ����
 */
class BaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit BaseWorker(QObject* parent = nullptr);
    virtual ~BaseWorker();

    /**
     * @brief ��ʼ�� Worker
     * @return �Ƿ��ʼ���ɹ�
     */
    virtual bool initialize() = 0;

    /**
     * @brief ���� Worker ��Դ
     */
    virtual void cleanup() {}

signals:
    /**
     * @brief Worker ��ʼ������ź�
     */
    void initialized();

    /**
     * @brief Worker �����ź�
     */
    void errorOccurred(const QString& error);

    /**
     * @brief Worker ״̬�仯�ź�
     */
    void statusChanged(const QString& status);

protected:
    bool m_initialized;
};

#endif // BASE_WORKER_H
