#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger
{
public:
    // 日志等级（严重 -> 啰嗦）
    enum class Level {
        Error = 0,  // 只看严重错误
        Warn = 1,  // 错误 + 警告
        Info = 2,  // 错误 + 警告 + 信息
        Debug = 3   // 全部（含调试信息）
    };

    // 单例
    static Logger& getInstance();

    // 初始化日志文件
    void init(const QString& filename = QStringLiteral("lanchat.log"));

    // 设置 / 获取 当前日志等级
    void setLevel(Level level);
    Level level() const;

    // 三种等级的接口（调用方式保持不变）
    void warning(const QString& message);
    void log(const QString& message);
    void error(const QString& message);

    void close();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 实际写入日志
    void write(const QString& levelName, const QString& message);

    // 判断某条日志在当前等级下是否需要写入
    bool shouldLog(Level msgLevel) const;

private:
    QFile logFile;
    Level m_level;
    QMutex m_mutex; // 保护文件写入的互斥锁
};

#endif // LOGGER_H
