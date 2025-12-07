#include "logger.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
Logger::Logger()
    : logFile()
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

void Logger::write(const QString& levelName, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime()
        .toString("yyyy-MM-dd HH:mm:ss");
    QString logLine = QString("[%1] %2: %3")
        .arg(timestamp, levelName, message);

    // VS �������
    qDebug().noquote() << logLine;
    // ��
    static QMutex s_mutex;
    QMutexLocker locker(&s_mutex);
    // �ļ�
    if (logFile.isOpen()) {
        QTextStream out(&logFile);
        out << logLine << "\n";
        logFile.flush();
    }
}

void Logger::warning(const QString& message)
{
    write("WARNING", message);
}

void Logger::log(const QString& message)
{
    write("INFO", message);
}

void Logger::error(const QString& message)
{
    write("ERROR", message);
}

void Logger::close()
{
    if (logFile.isOpen()) {
        logFile.close();
    }
}

