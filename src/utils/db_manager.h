#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QMutex>
#include <QString>
#include <QSqlDatabase>
#include <QMap>
class DatabaseManager
{
public:
    static DatabaseManager& getInstance();

    // 在 main() 启动时调用一次；可传入自定义 dbPath（如 ":memory:" 用于测试）
    bool init(const QString& dbFilePath = QString());

    // 新增：初始化额外数据库连接
    bool initConnection(const QString& connectionName, const QString& dbFilePath);

    // 在当前线程返回 QSqlDatabase（首次调用会创建连接）
    QSqlDatabase database();

    // 新增：获取指定名称的数据库连接
    QSqlDatabase database(const QString& connectionName);

    // 执行简单 SQL（建表、PRAGMA 等）
    bool exec(const QString& sql);

    // 新增：在指定数据库连接上执行 SQL
    bool exec(const QString& sql, const QString& connectionName);

    // 事务接口
    bool beginTransaction();
    bool commit();
    bool rollback();

    // 新增：在指定数据库连接上执行事务
    bool beginTransaction(const QString& connectionName);
    bool commit(const QString& connectionName);
    bool rollback(const QString& connectionName);

    // 关闭并移除当前线程连接（线程退出时调用）
    void closeConnectionForCurrentThread();

    // 新增：关闭指定连接名的线程连接
    void closeConnectionForThread(const QString& connectionName);

    // 建表与 schema 初始化
    bool ensureSchema();

    // 新增：在指定数据库上执行 schema 初始化
    bool ensureSchema(const QString& connectionName);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 生成连接名：保持原有逻辑，增加 connectionName 参数
    QString connectionNameForCurrentThread(const QString& baseConnectionName = QString()) const;
    // 内部：获取或创建数据库连接
    QSqlDatabase getOrCreateConnection(const QString& baseConnectionName, const QString& dbFilePath);

    /*QString connectionNameForCurrentThread() const;*/
    QString defaultDbPath() const;

    QString m_defaultConnectionName = "lanchat";  // 默认连接名
    QString m_dbPath;  // 默认数据库路径
    QMutex m_mutex;  // 线程安全
    // 新增：连接名到文件路径的映射
    QMap<QString, QString> m_connectionPaths;
};

#endif // DB_MANAGER_H