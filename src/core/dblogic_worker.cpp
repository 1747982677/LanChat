#include "dblogic_worker.h"
#include "utils/db_manager.h"
#include <QJsonArray>
#include <QDebug>
#include <QThread>

DbLogicWorker::DbLogicWorker(QObject* parent)
    : BaseWorker(parent)
    , m_dbInitialized(false)
{
}

DbLogicWorker::~DbLogicWorker()
{
    cleanup();
}

bool DbLogicWorker::initialize()
{
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    emit initialized();
    emit statusChanged("DbLogic Worker initialized");
    
    qDebug() << "DbLogicWorker initialized";
    return true;
}

void DbLogicWorker::cleanup()
{
    if (!m_initialized) {
        return;
    }

    if (m_dbInitialized) {
        DatabaseManager::getInstance().closeConnectionForCurrentThread();
        m_dbInitialized = false;
    }

    m_initialized = false;
    qDebug() << "DbLogicWorker cleaned up";
}

void DbLogicWorker::initializeDatabase(const QString& dbPath)
{
    qDebug() << "Initializing database:" << dbPath;
    
    m_dbPath = dbPath;
    bool success = DatabaseManager::getInstance().init(dbPath);
    m_dbInitialized = success;
    
    if (success) {
        qDebug() << "Database initialized successfully";
    } else {
        qDebug() << "Failed to initialize database";
        emit errorOccurred("Failed to initialize database");
    }
    
    emit databaseInitialized(success);
}

void DbLogicWorker::saveMessage(const QJsonObject& message)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit messageSaved(false, message.value("messageId").toString());
        return;
    }

    qDebug() << "Saving message to database:" << message;
    
    QString messageId = message.value("messageId").toString();
    
    // TODO: 实际的数据库保存逻辑
    bool success = true;
    
    emit messageSaved(success, messageId);
}

//  修复：添加 requestId 参数
void DbLogicWorker::loadHistoryMessages(const QString& requestId, const QString& contactId, int limit, int offset)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit historyMessagesLoaded(requestId, QJsonArray(), contactId);
        return;
    }

    qDebug() << "Loading history messages for contact:" << contactId 
             << "requestId:" << requestId
             << "limit:" << limit << "offset:" << offset;
    
    // 模拟耗时查询
    QThread::msleep(200);  // 模拟 200ms 的查询时间
    
    // TODO: 实际的数据库查询逻辑
    QJsonArray messages;
    
    //  修复：返回时携带 requestId
    emit historyMessagesLoaded(requestId, messages, contactId);
}

void DbLogicWorker::searchMessages(const QString& keyword)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit searchResultsReady(QJsonArray());
        return;
    }

    qDebug() << "Searching messages with keyword:" << keyword;
    
    // TODO: 实际的搜索逻辑
    QJsonArray results;
    
    emit searchResultsReady(results);
}

void DbLogicWorker::updateMessageStatus(const QString& messageId, const QString& status)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit messageStatusUpdated(false, messageId);
        return;
    }

    qDebug() << "Updating message status:" << messageId << "to" << status;
    
    // TODO: 实际的状态更新逻辑
    bool success = true;
    
    emit messageStatusUpdated(success, messageId);
}

void DbLogicWorker::deleteMessage(const QString& messageId)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        return;
    }

    qDebug() << "Deleting message:" << messageId;
    
    // TODO: 实际的删除逻辑
}

void DbLogicWorker::loadContactList()
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit contactListLoaded(QJsonArray());
        return;
    }

    qDebug() << "Loading contact list";
    
    // TODO: 实际的联系人列表加载逻辑
    QJsonArray contacts;
    
    emit contactListLoaded(contacts);
}

void DbLogicWorker::addContact(const QJsonObject& contactInfo)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit contactOperationCompleted(false, "add");
        return;
    }

    qDebug() << "Adding contact:" << contactInfo;
    
    // TODO: 实际的添加联系人逻辑
    bool success = true;
    
    emit contactOperationCompleted(success, "add");
}

void DbLogicWorker::updateContact(const QString& contactId, const QJsonObject& contactInfo)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit contactOperationCompleted(false, "update");
        return;
    }

    qDebug() << "Updating contact:" << contactId << contactInfo;
    
    // TODO: 实际的更新联系人逻辑
    bool success = true;
    
    emit contactOperationCompleted(success, "update");
}

void DbLogicWorker::processFile(const QString& filePath, const QJsonObject& options)
{
    qDebug() << "Processing file:" << filePath << "with options:" << options;
    
    // TODO: 实际的文件处理逻辑
    QString resultPath = filePath;
    bool success = true;
    
    emit fileProcessed(success, filePath, resultPath);
}
