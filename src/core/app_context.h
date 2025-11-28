#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <QObject>
#include "core/network_controller.h"
#include "core/dblogic_controller.h"

/**
 * @brief 全局应用上下文管理器
 * 
 * 单例模式，负责：
 * 1. 统一管理所有 Controller 实例
 * 2. 提供全局访问接口
 * 3. 协调各个模块之间的交互
 * 4. 管理应用的生命周期
 */
class AppContext : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例
     */
    static AppContext& instance();

    /**
     * @brief 初始化应用上下文
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 启动所有模块
     */
    void startAll();

    /**
     * @brief 停止所有模块
     */
    void stopAll();

    /**
     * @brief 获取网络控制器
     */
    NetworkController* networkController();

    /**
     * @brief 获取数据库逻辑控制器
     */
    DbLogicController* dbLogicController();

    /**
     * @brief 设置数据库路径
     */
    void setDatabasePath(const QString& path);

    /**
     * @brief 获取数据库路径
     */
    QString databasePath() const;

signals:
    /**
     * @brief 应用初始化完成
     */
    void applicationInitialized();

    /**
     * @brief 应用错误信号
     */
    void applicationError(const QString& error);

private:
    explicit AppContext(QObject* parent = nullptr);
    ~AppContext();
    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    void connectControllers();

    static AppContext* s_instance;

    NetworkController* m_networkController;
    DbLogicController* m_dbLogicController;

    QString m_databasePath;
    bool m_initialized;
};

#endif // APP_CONTEXT_H
