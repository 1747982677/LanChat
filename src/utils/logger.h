#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class Logger {
public:
    // 单例
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // 初始化日志文件
    void init(const QString& filename = "lanchat.log");

    // 三种等级的接口（都一定会写）
    void warning(const QString& message);
    void log(const QString& message);
    void error(const QString& message);

    void close();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void write(const QString& levelName, const QString& message);

private:
    QFile logFile;
};

#endif // LOGGER_H
