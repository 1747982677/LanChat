#ifndef BASE_WORKER_H
#define BASE_WORKER_H

#include <QObject>

/**
 * @brief Worker 基类
 * 
 * 所有 Worker 的基类，运行在独立线程中
 * Worker 负责执行具体的耗时任务
 */
class BaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit BaseWorker(QObject* parent = nullptr);
    virtual ~BaseWorker();

    /**
     * @brief 初始化 Worker
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 清理 Worker 资源
     */
    virtual void cleanup() = 0;

signals:
    /**
     * @brief Worker 初始化完成信号
     */
    void initialized();

    /**
     * @brief Worker 错误信号
     */
    void errorOccurred(const QString& error);

    /**
     * @brief Worker 状态变化信号
     */
    void statusChanged(const QString& status);

protected:
    bool m_initialized;
};

#endif // BASE_WORKER_H
