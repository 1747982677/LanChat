#include "dblogic_worker.h"
#include "utils/db_manager.h"
#include "model/message_dao.h"
#include "model/message.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QMap>

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
    
    // TODO: ʵ�ʵ����ݿⱣ���߼�
    bool success = true;
    
    emit messageSaved(success, messageId);
}

//  �޸������� requestId ����
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
    
    // ģ���ʱ��ѯ
    QThread::msleep(200);  // ģ�� 200ms �Ĳ�ѯʱ��
    
    // TODO: ʵ�ʵ����ݿ��ѯ�߼�
    QJsonArray messages;
    
    //  �޸�������ʱЯ�� requestId
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
    
    // TODO: ʵ�ʵ������߼�
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
    
    // TODO: ʵ�ʵ�״̬�����߼�
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
    
    // TODO: ʵ�ʵ�ɾ���߼�
}

void DbLogicWorker::loadContactList()
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit contactListLoaded(QJsonArray());
        return;
    }

    qDebug() << "Loading contact list";
    
    // TODO: ʵ�ʵ���ϵ���б������߼�
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
    
    // TODO: ʵ�ʵ�������ϵ���߼�
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
    
    // TODO: ʵ�ʵĸ�����ϵ���߼�
    bool success = true;
    
    emit contactOperationCompleted(success, "update");
}

void DbLogicWorker::searchUserByAccount(const QString& account)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit userSearchResult(QJsonObject(), false);
        return;
    }
    
    qDebug() << "Searching user by account:" << account;
    
    if (account.isEmpty()) {
        emit userSearchResult(QJsonObject(), false);
        return;
    }
    
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit userSearchResult(QJsonObject(), false);
        return;
    }
    
    // 精确匹配账号（不支持模糊查询）
    QSqlQuery q(db);
    q.prepare("SELECT userId, account, nickname, avatarPath, email, phone, signature, status FROM users WHERE account = :account");
    q.bindValue(":account", account);
    
    if (!q.exec()) {
        qDebug() << "Search user failed:" << q.lastError().text();
        emit errorOccurred("Search user failed: " + q.lastError().text());
        emit userSearchResult(QJsonObject(), false);
        return;
    }
    
    if (q.next()) {
        // 找到用户，构造 UserInfo JSON
        QJsonObject userInfo;
        userInfo["userId"] = q.value("userId").toString();
        userInfo["account"] = q.value("account").toString();
        userInfo["nickname"] = q.value("nickname").toString();
        userInfo["avatarPath"] = q.value("avatarPath").toString();
        userInfo["email"] = q.value("email").toString();
        userInfo["phone"] = q.value("phone").toString();
        userInfo["signature"] = q.value("signature").toString();
        userInfo["status"] = q.value("status").toInt();
        
        qDebug() << "User found:" << userInfo["account"].toString();
        emit userSearchResult(userInfo, true);
    } else {
        qDebug() << "User not found with account:" << account;
        emit userSearchResult(QJsonObject(), false);
    }
}

void DbLogicWorker::processFile(const QString& filePath, const QJsonObject& options)
{
    qDebug() << "Processing file:" << filePath << "with options:" << options;
    
    // TODO: ʵ�ʵ��ļ������߼�
    QString resultPath = filePath;
    bool success = true;
    
    emit fileProcessed(success, filePath, resultPath);
}
