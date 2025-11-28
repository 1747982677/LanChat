#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void init(const QString& filename = "lanchat.log");
    void log(const QString& message);
    void error(const QString& message);
    void warning(const QString& message);
    void close();
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void writeLog(const QString& level, const QString& message);
    
    QFile logFile;
    QTextStream* stream;
    QMutex mutex;
    bool initialized;
};

#endif // LOGGER_H
