#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QString>
#include <QSqlDatabase>

class DatabaseManager
{
public:
    static DatabaseManager& getInstance();

    // 在 main() 启动时调用一次；可传入自定义 dbPath（如 ":memory:" 用于测试）
    bool init(const QString& dbFilePath = QString());

    // 在当前线程返回 QSqlDatabase（首次调用会创建连接）
    QSqlDatabase database();

    // 执行简单 SQL（建表、PRAGMA 等）
    bool exec(const QString& sql);

    // 事务接口
    bool beginTransaction();
    bool commit();
    bool rollback();

    // 关闭并移除当前线程连接（线程退出时调用）
    void closeConnectionForCurrentThread();

    // 建表与 schema 初始化
    bool ensureSchema();

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QString connectionNameForCurrentThread() const;
    QString defaultDbPath() const;

    QString m_dbPath;
};

#endif // DB_MANAGER_H