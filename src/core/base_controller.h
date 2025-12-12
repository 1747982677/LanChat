#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <QObject>
#include <QThread>

/**
 * @brief 控制器基类
 * 
 * 所有 Controller 的基类，负责管理 Worker 线程的生命周期
 * Controller 运行在主线程，通过信号槽与 Worker 通信
 */
class BaseController : public QObject
{
    Q_OBJECT

public:
    explicit BaseController(QObject* parent = nullptr);
    virtual ~BaseController();

    /**
     * @brief 初始化控制器
     * @return 是否初始化成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 启动 Worker 线程
     */
    virtual void start();

    /**
     * @brief 停止 Worker 线程
     */
    virtual void stop();

    /**
     * @brief 检查线程是否正在运行
     */
    bool isRunning() const;

signals:
    /**
     * @brief 控制器初始化完成信号
     */
    void initialized();

    /**
     * @brief 控制器错误信号
     */
    void errorOccurred(const QString& error);

protected:
    /**
     * @brief 创建 Worker 实例
     * 子类必须实现此方法以创建对应的 Worker
     */
    virtual QObject* createWorker() = 0;

    /**
     * @brief 设置 Worker 和 Thread
     */
    void setupWorkerThread(QObject* worker);

    QThread* m_workerThread;  // Worker 线程
    QObject* m_worker;        // Worker 对象（运行在 Worker 线程中）

private:
    bool m_isRunning;
};

#endif // BASE_CONTROLLER_H
