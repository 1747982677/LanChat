#include "logger.h"
#include <QDebug>
#include <QIODevice>

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : logFile()
    , m_level(Level::Info)  // 默认 Info：ERROR/WARN/INFO 都会输出
{
}

Logger::~Logger()
{
    close();
}

void Logger::init(const QString& filename)
{
    if (logFile.isOpen()) {
        logFile.close();
    }

    logFile.setFileName(filename);

    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qDebug() << "[LOGGER] Failed to open log file:" << filename;
    }
    else {
        qDebug() << "[LOGGER] Log file opened at:" << filename;
    }
}

void Logger::setLevel(Level level)
{
    m_level = level;
}

Logger::Level Logger::level() const
{
    return m_level;
}

bool Logger::shouldLog(Level msgLevel) const
{
    // 数值越小越严重（Error=0），当前等级是“最低严重级别”
    // msgLevel <= m_level 时记录
    return static_cast<int>(msgLevel) <= static_cast<int>(m_level);
}

void Logger::write(const QString& levelName, const QString& message)
{
    const QString timestamp = QDateTime::currentDateTime()
        .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    const QString logLine = QStringLiteral("[%1] %2: %3")
        .arg(timestamp, levelName, message);

    // 控制台 / Qt Creator 输出窗口
    qDebug().noquote() << logLine;

    // 写入文件
    if (logFile.isOpen()) {
        QTextStream out(&logFile);
        out << logLine << '\n';
        logFile.flush();
    }
}

void Logger::warning(const QString& message)
{
    if (!shouldLog(Level::Warn)) {
        return;
    }
    write(QStringLiteral("WARNING"), message);
}

void Logger::log(const QString& message)
{
    if (!shouldLog(Level::Info)) {
        return;
    }
    write(QStringLiteral("INFO"), message);
}

void Logger::error(const QString& message)
{
    if (!shouldLog(Level::Error)) {
        return;
    }
    write(QStringLiteral("ERROR"), message);
}

void Logger::close()
{
    if (logFile.isOpen()) {
        logFile.close();
    }
}
