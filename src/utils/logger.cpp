#include "logger.h"
#include <QDebug>
#include <QMutexLocker>

Logger::Logger() : initialized(false), stream(nullptr) {} // Initialize member variables

void Logger::init(const QString& filename) {
    QMutexLocker locker(&mutex);
    
    logFile.setFileName(filename);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        stream = new QTextStream(&logFile);
        initialized = true;
        log("Logger initialized");
    } else {
        qWarning() << "Failed to open log file:" << filename;
        stream = nullptr;
        initialized = false;
    }
}

Logger::~Logger() {
    close();
}

void Logger::close() {
    QMutexLocker locker(&mutex);
    
    if (stream) {
        delete stream;
        stream = nullptr;
    }
    
    if (logFile.isOpen()) {
        logFile.close();
    }
    
    initialized = false;
}

void Logger::writeLog(const QString& level, const QString& message) {
    QMutexLocker locker(&mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString logMessage = QString("[%1] %2: %3").arg(timestamp, level, message);
    
    if (initialized && stream) {
        *stream << logMessage << Qt::endl;
        stream->flush();
    }
    
    // Also output to console
    qDebug().noquote() << logMessage;
}

void Logger::log(const QString& message) {
    writeLog("INFO", message);
}

void Logger::error(const QString& message) {
    writeLog("ERROR", message);
}

void Logger::warning(const QString& message) {
    writeLog("WARNING", message);
}
