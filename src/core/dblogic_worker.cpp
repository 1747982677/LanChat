#include "dblogic_worker.h"
#include "utils/db_manager.h"
#include "utils/password_util.h"
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
#include <QUuid>

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
    
    // 精确匹配邮箱（账号就是邮箱，不支持模糊查询）
    QSqlQuery q(db);
    q.prepare("SELECT userId, email, nickname, avatarPath, phone, signature, status FROM users WHERE email = :email");
    q.bindValue(":email", account);  // account 参数实际是邮箱地址
    
    if (!q.exec()) {
        qDebug() << "Search user failed:" << q.lastError().text();
        emit errorOccurred("Search user failed: " + q.lastError().text());
        emit userSearchResult(QJsonObject(), false);
        return;
    }
    
    if (q.next()) {
        // 找到用户，构造 UserInfo JSON
        QString email = q.value("email").toString();
        QJsonObject userInfo;
        userInfo["userId"] = q.value("userId").toString();
        userInfo["account"] = email;  // 账号就是邮箱
        userInfo["nickname"] = q.value("nickname").toString();
        userInfo["avatarPath"] = q.value("avatarPath").toString();
        userInfo["email"] = email;  // 邮箱字段
        userInfo["phone"] = q.value("phone").toString();
        userInfo["signature"] = q.value("signature").toString();
        userInfo["status"] = q.value("status").toInt();
        
        qDebug() << "User found:" << email;
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

void DbLogicWorker::registerUser(const QString& email, const QString& passwordHash)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit userRegistered(false, QString(), "数据库未初始化");
        return;
    }

    qDebug() << "Registering user:" << email;

    if (email.isEmpty() || passwordHash.isEmpty()) {
        emit userRegistered(false, QString(), "邮箱或密码不能为空");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit userRegistered(false, QString(), "数据库未打开");
        return;
    }

    // 检查邮箱是否已存在
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT userId FROM users WHERE email = :email");
    checkQuery.bindValue(":email", email);
    
    if (!checkQuery.exec()) {
        qDebug() << "Check user existence failed:" << checkQuery.lastError().text();
        emit errorOccurred("Check user existence failed: " + checkQuery.lastError().text());
        emit userRegistered(false, QString(), "检查用户是否存在时出错");
        return;
    }

    if (checkQuery.next()) {
        qDebug() << "User already exists:" << email;
        emit userRegistered(false, QString(), "该邮箱已被注册");
        return;
    }

    // 生成用户ID
    QString userId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 插入新用户（昵称字段留空，用户可以在后续设置）
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users (userId, email, passwordHash, nickname, status, lastOnlineTime) "
                        "VALUES (:userId, :email, :passwordHash, :nickname, :status, :lastOnlineTime)");
    insertQuery.bindValue(":userId", userId);
    insertQuery.bindValue(":email", email);
    insertQuery.bindValue(":passwordHash", passwordHash);
    insertQuery.bindValue(":nickname", QString());  // 昵称留空，不自动生成
    insertQuery.bindValue(":status", 0);  // 默认离线状态
    insertQuery.bindValue(":lastOnlineTime", QDateTime::currentSecsSinceEpoch());

    if (!insertQuery.exec()) {
        qDebug() << "Register user failed:" << insertQuery.lastError().text();
        emit errorOccurred("Register user failed: " + insertQuery.lastError().text());
        emit userRegistered(false, QString(), "注册失败: " + insertQuery.lastError().text());
        return;
    }

    qDebug() << "User registered successfully:" << email << "userId:" << userId;
    emit userRegistered(true, userId, QString());
}

void DbLogicWorker::verifyUserPassword(const QString& email, const QString& password)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit passwordVerified(false, QString(), "数据库未初始化");
        return;
    }

    qDebug() << "Verifying password for user:" << email;

    if (email.isEmpty() || password.isEmpty()) {
        emit passwordVerified(false, QString(), "邮箱或密码不能为空");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit passwordVerified(false, QString(), "数据库未打开");
        return;
    }

    // 查询用户信息（包括密码哈希）
    QSqlQuery q(db);
    q.prepare("SELECT userId, passwordHash FROM users WHERE email = :email");
    q.bindValue(":email", email);

    if (!q.exec()) {
        qDebug() << "Query user failed:" << q.lastError().text();
        emit errorOccurred("Query user failed: " + q.lastError().text());
        emit passwordVerified(false, QString(), "查询用户失败");
        return;
    }

    if (!q.next()) {
        qDebug() << "User not found:" << email;
        emit passwordVerified(false, QString(), "用户不存在");
        return;
    }

    QString userId = q.value("userId").toString();
    QString storedPasswordHash = q.value("passwordHash").toString();

    // 验证密码
    bool isValid = PasswordUtil::verifyPassword(password, storedPasswordHash);
    
    if (isValid) {
        qDebug() << "Password verified successfully for user:" << email;
        emit passwordVerified(true, userId, QString());
    } else {
        qDebug() << "Password verification failed for user:" << email;
        emit passwordVerified(false, QString(), "密码错误");
    }
}
