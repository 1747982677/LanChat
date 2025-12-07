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
#include "ui/personinfo/UserEntity.h"
#include <QSqlRecord>
#include <QFileInfo>
#include <QDir>

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

    // 确保数据库文件所在的目录存在
    QFileInfo fileInfo(dbPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        qDebug() << "Database directory does not exist, creating:" << dir.absolutePath();
        if (!dir.mkpath(".")) {
            qDebug() << "Failed to create database directory:" << dir.absolutePath();
            emit errorOccurred("Failed to create database directory: " + dir.absolutePath());
            emit databaseInitialized(false);
            return;
        }
    }

    m_dbPath = dbPath;

    // 初始化 lanchat 数据库（用于消息和用户认证）
    bool success = DatabaseManager::getInstance().init(dbPath);

    if (!success) {
        qDebug() << "Failed to initialize lanchat database";
        emit errorOccurred("Failed to initialize lanchat database");
        emit databaseInitialized(false);
        return;
    }

    qDebug() << "Lanchat database initialized successfully";

    // 初始化 public 数据库（用于用户信息管理）
    // 从 dbPath 推导 public.db 的路径（在同一目录下）
    QFileInfo lanchatInfo(dbPath);
    QString publicDbPath = lanchatInfo.absoluteDir().absoluteFilePath("public.db");

    // 确保 public.db 目录存在
    QFileInfo publicInfo(publicDbPath);
    QDir publicDir = publicInfo.absoluteDir();
    if (!publicDir.exists()) {
        qDebug() << "Public database directory does not exist, creating:" << publicDir.absolutePath();
        if (!publicDir.mkpath(".")) {
            qDebug() << "Failed to create public database directory:" << publicDir.absolutePath();
            emit errorOccurred("Failed to create public database directory: " + publicDir.absolutePath());
            emit databaseInitialized(false);
            return;
        }
    }

    qDebug() << "Initializing public database:" << publicDbPath;
    bool success2 = DatabaseManager::getInstance().initConnection("public", publicDbPath);

    if (!success2) {
        qDebug() << "Failed to initialize public database";
        emit errorOccurred("Failed to initialize public database");
        emit databaseInitialized(false);
        return;
    }

    qDebug() << "Public database initialized successfully";

    // 两个数据库都初始化成功才算成功
    m_dbInitialized = success && success2;

    emit databaseInitialized(m_dbInitialized);
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

    // TODO: ʵ ʵ    ݿⱣ   ߼ 
    bool success = true;

    emit messageSaved(success, messageId);
}

//   ޸        requestId     
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

    // ģ   ʱ  ѯ
    QThread::msleep(200);  // ģ   200ms  Ĳ ѯʱ  

    // TODO: ʵ ʵ    ݿ  ѯ ߼ 
    QJsonArray messages;

    //   ޸       ʱЯ   requestId
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

    // TODO: ʵ ʵ      ߼ 
    QJsonArray results;

    emit searchResultsReady(results);
}

void DbLogicWorker::queryMessages(const QString& localUser, const QString& peer, int limit)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit searchResultsReady(QJsonArray());
        return;
    }
    QVector<Message> conv = MessageDao::getConversation("alice", "bob", 100);
    /*return conv;*/
    qDebug() << "Searching messages with keyword:" << localUser;
    emit queryResultsReady(conv);
}

void DbLogicWorker::updateUser(const UserEntity& localUser)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit updateUserReady(false);
        return;
    }
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database("public");
    QSqlQuery query(db);
    QString sql = localUser.toUpdateSQL();
    query.prepare(sql);

    if (!query.exec()) {
        qWarning() << "查询用户失败:" << query.lastError();
        emit updateUserReady(false);
        return;
    }
    else {
        emit updateUserReady(true);
    }

}

void DbLogicWorker::addUser(const UserEntity& localUser)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit addUserReady(false);
        return;
    }
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database("public");
    QSqlQuery query(db);
    QString sql = localUser.toInsertSQL();
    query.prepare(sql);

    if (!query.exec()) {
        qWarning() << "查询用户失败:" << query.lastError();
        emit addUserReady(false);
        return;
    }
    else {
        emit addUserReady(true);
    }
}


void DbLogicWorker::queryUser(const UserEntity& localUser)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit queryUserReady(UserEntity());
        return;
    }
    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database("public");
    QSqlQuery query(db);
    QString sql = localUser.selectByUserIdSQL(localUser.userId);
    query.prepare(sql);

    if (!query.exec()) {
        qWarning() << "查询用户失败:" << query.lastError();
        return;
    }
    if (query.next()) {
        // 将查询结果转换为 QMap
        QMap<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            row[record.fieldName(i)] = record.value(i);
        }
        emit queryUserReady(UserEntity::fromDatabase(row));
    }
    else {
        emit queryUserReady(UserEntity());
    }
}


void DbLogicWorker::updateMessageStatus(const QString& messageId, const QString& status)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit messageStatusUpdated(false, messageId);
        return;
    }

    qDebug() << "Updating message status:" << messageId << "to" << status;

    // TODO: ʵ ʵ ״̬     ߼ 
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

    // TODO: ʵ ʵ ɾ   ߼ 
}
void DbLogicWorker::clearAllChatHistory()
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit allChatHistoryCleared(false, "数据库未初始化");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();  // 默认是 lanchat.db
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit allChatHistoryCleared(false, "数据库未打开");
        return;
    }

    qDebug() << "Clearing ALL chat history";


    if (!db.transaction()) {
        emit allChatHistoryCleared(false, "开始事务失败");
        return;
    }

    QSqlQuery query(db);

    // 这里根据你实际的消息表名来改：
    // 假设表名叫 messages
    if (!query.exec("DELETE FROM messages")) {
        qWarning() << "Clear ALL chat history failed:" << query.lastError();
        db.rollback();
        emit errorOccurred("Clear ALL chat history failed: " + query.lastError().text());
        emit allChatHistoryCleared(false, "清空聊天记录失败: " + query.lastError().text());
        return;
    }

    // 如果你希望连“好友请求里的聊天记录/通知”也清，可以在这里再执行一条：
    // QSqlQuery query2(db);
    // query2.exec("DELETE FROM friend_requests");

    if (!db.commit()) {
        db.rollback();
        emit allChatHistoryCleared(false, "提交事务失败");
        return;
    }

    qDebug() << "ALL chat history cleared successfully";
    emit allChatHistoryCleared(true, QString());
}
void DbLogicWorker::calculateMessagesTableSize()
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit messagesTableSizeCalculated(false, 0, "数据库未初始化");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();  // lanchat.db
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit messagesTableSizeCalculated(false, 0, "数据库未打开");
        return;
    }

    qDebug() << "[DbLogicWorker] Calculating messages table size (sum of all columns)...";

    QSqlQuery q(db);

    const char* sql =
        "SELECT "
        "  IFNULL(SUM(LENGTH(CAST(id AS BLOB))), 0) + "
        "  IFNULL(SUM(LENGTH(sender)), 0) + "
        "  IFNULL(SUM(LENGTH(receiver)), 0) + "
        "  IFNULL(SUM(LENGTH(content)), 0) + "
        "  IFNULL(SUM(LENGTH(CAST(timestamp AS BLOB))), 0) + "
        "  IFNULL(SUM(LENGTH(CAST(status AS BLOB))), 0) + "
        "  IFNULL(SUM(LENGTH(extra)), 0) "
        "FROM messages";

    if (!q.exec(sql)) {
        qWarning() << "[DbLogicWorker] calculateMessagesTableSize failed:" << q.lastError();
        emit messagesTableSizeCalculated(false, 0, "无法统计 messages 表大小");
        return;
    }

    if (!q.next()) {
        qWarning() << "[DbLogicWorker] calculateMessagesTableSize: no result row";
        emit messagesTableSizeCalculated(false, 0, "无法获取 messages 表大小");
        return;
    }

    qint64 sizeBytes = q.value(0).toLongLong();
    qDebug() << "[DbLogicWorker] messages logical size =" << sizeBytes << "bytes";

    emit messagesTableSizeCalculated(true, sizeBytes, QString());
}



void DbLogicWorker::loadContactList(const QString& userId)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit contactListLoaded(QJsonArray());
        return;
    }

    qDebug() << "Loading contact list for user:" << userId;

    if (userId.isEmpty()) {
        qDebug() << "UserId is empty, returning empty contact list";
        emit contactListLoaded(QJsonArray());
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase lanchatDb = dbm.database();
    QSqlDatabase publicDb = dbm.database("public");

    if (!lanchatDb.isOpen() || !publicDb.isOpen()) {
        qDebug() << "Database not open";
        emit errorOccurred("Database not open");
        emit contactListLoaded(QJsonArray());
        return;
    }

    // 先查询数据库中所有的好友关系（用于调试）
    QSqlQuery debugQuery(lanchatDb);
    debugQuery.prepare("SELECT userId, friendId FROM friends");
    if (debugQuery.exec()) {
        qDebug() << "=== All friend relationships in database ===";
        while (debugQuery.next()) {
            qDebug() << "  userId:" << debugQuery.value("userId").toString()
                << "-> friendId:" << debugQuery.value("friendId").toString();
        }
    }

    qDebug() << "=== Querying contacts for userId:" << userId << "===";

    QSqlQuery friendQuery(lanchatDb);
    friendQuery.prepare("SELECT friendId, remark, addedTime FROM friends WHERE userId = :userId ORDER BY addedTime DESC");
    friendQuery.bindValue(":userId", userId);

    if (!friendQuery.exec()) {
        qDebug() << "Query friends failed:" << friendQuery.lastError().text();
        emit errorOccurred("Query friends failed: " + friendQuery.lastError().text());
        emit contactListLoaded(QJsonArray());
        return;
    }

    QJsonArray contacts;
    int count = 0;
    while (friendQuery.next()) {
        count++;
        QString friendId = friendQuery.value("friendId").toString();
        QString remark = friendQuery.value("remark").toString();

        QSqlQuery userQuery(publicDb);
        userQuery.prepare("SELECT userId, email, nickname, avatarPath, status FROM users WHERE userId = :friendId");
        userQuery.bindValue(":friendId", friendId);

        QJsonObject contact;
        contact["friendId"] = friendId;
        contact["remark"] = remark;

        if (userQuery.exec() && userQuery.next()) {
            QString nickname = userQuery.value("nickname").toString();
            QString email = userQuery.value("email").toString();
            QString avatarPath = userQuery.value("avatarPath").toString();
            int status = userQuery.value("status").toInt();

            QString displayName = remark.isEmpty() ? nickname : remark;
            if (displayName.isEmpty()) {
                displayName = email;
            }
            if (displayName.isEmpty()) {
                displayName = friendId;
            }

            contact["nickname"] = nickname;
            contact["email"] = email;
            contact["avatarPath"] = avatarPath;
            contact["displayName"] = displayName;
            contact["status"] = status;
        }
        else {
            QString displayName = remark.isEmpty() ? friendId : remark;
            contact["displayName"] = displayName;
            contact["nickname"] = QString();
            contact["email"] = QString();
            contact["avatarPath"] = QString();
            contact["status"] = 0;
        }

        contacts.append(contact);
        qDebug() << "  Added contact #" << count << ":" << contact["displayName"].toString()
            << "(friendId:" << friendId << ", email:" << contact["email"].toString() << ")";
    }

    qDebug() << "=== Loaded" << contacts.size() << "contacts for userId:" << userId << "===";
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

    // TODO: ʵ ʵ       ϵ   ߼ 
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

    // TODO: ʵ ʵĸ     ϵ   ߼ 
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
    QSqlDatabase db = dbm.database("public");  // 使用 public 数据库连接
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
    }
    else {
        qDebug() << "User not found with account:" << account;
        emit userSearchResult(QJsonObject(), false);
    }
}

void DbLogicWorker::processFile(const QString& filePath, const QJsonObject& options)
{
    qDebug() << "Processing file:" << filePath << "with options:" << options;

    // TODO: ʵ ʵ  ļ      ߼ 
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
    // 使用 public.db 存储用户账号信息（作为中央服务器）
    QSqlDatabase db = dbm.database("public");
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
    // 从 public.db 查询用户账号信息（作为中央服务器）
    QSqlDatabase db = dbm.database("public");
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
    }
    else {
        qDebug() << "Password verification failed for user:" << email;
        emit passwordVerified(false, QString(), "密码错误");
    }
}

void DbLogicWorker::sendFriendRequest(const QString& senderId, const QString& receiverId,
    const QString& senderAccount, const QString& senderNickname,
    const QString& avatarPath, const QString& verifymsg)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit friendRequestSent(false, QString(), "数据库未初始化");
        return;
    }

    qDebug() << "Sending friend request from" << senderId << "to" << receiverId;

    if (senderId.isEmpty() || receiverId.isEmpty()) {
        emit friendRequestSent(false, QString(), "发送者或接收者ID不能为空");
        return;
    }

    // 不能给自己发送好友请求
    if (senderId == receiverId) {
        emit friendRequestSent(false, QString(), "不能给自己发送好友请求");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();  // 使用 lanchat.db（本地存储）
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit friendRequestSent(false, QString(), "数据库未打开");
        return;
    }

    // 检查是否已经是好友
    QSqlQuery checkFriendQuery(db);
    checkFriendQuery.prepare("SELECT id FROM friends WHERE (userId = :userId1 AND friendId = :userId2) OR (userId = :userId2 AND friendId = :userId1)");
    checkFriendQuery.bindValue(":userId1", senderId);
    checkFriendQuery.bindValue(":userId2", receiverId);

    if (checkFriendQuery.exec() && checkFriendQuery.next()) {
        qDebug() << "Users are already friends";
        emit friendRequestSent(false, QString(), "你们已经是好友了");
        return;
    }

    // 检查是否已经发送过请求（且状态为 Pending）
    QSqlQuery checkRequestQuery(db);
    checkRequestQuery.prepare("SELECT requestId FROM friend_requests WHERE senderId = :senderId AND receiverId = :receiverId AND status = 0");
    checkRequestQuery.bindValue(":senderId", senderId);
    checkRequestQuery.bindValue(":receiverId", receiverId);

    if (checkRequestQuery.exec() && checkRequestQuery.next()) {
        qDebug() << "Friend request already sent";
        emit friendRequestSent(false, QString(), "已经发送过好友请求，请等待对方回应");
        return;
    }

    // 生成请求ID
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 插入好友请求
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO friend_requests (requestId, senderId, receiverId, senderAccount, senderNickname, avatarPath, verifymsg, status, timestamp) "
        "VALUES (:requestId, :senderId, :receiverId, :senderAccount, :senderNickname, :avatarPath, :verifymsg, :status, :timestamp)");
    insertQuery.bindValue(":requestId", requestId);
    insertQuery.bindValue(":senderId", senderId);
    insertQuery.bindValue(":receiverId", receiverId);
    insertQuery.bindValue(":senderAccount", senderAccount);
    insertQuery.bindValue(":senderNickname", senderNickname);
    insertQuery.bindValue(":avatarPath", avatarPath);
    insertQuery.bindValue(":verifymsg", verifymsg);
    insertQuery.bindValue(":status", 0);  // 0 = Pending
    insertQuery.bindValue(":timestamp", QDateTime::currentSecsSinceEpoch());

    if (!insertQuery.exec()) {
        qDebug() << "Send friend request failed:" << insertQuery.lastError().text();
        emit errorOccurred("Send friend request failed: " + insertQuery.lastError().text());
        emit friendRequestSent(false, QString(), "发送好友请求失败: " + insertQuery.lastError().text());
        return;
    }

    qDebug() << "Friend request sent successfully, requestId:" << requestId;
    emit friendRequestSent(true, requestId, QString());
}

void DbLogicWorker::queryFriendRequests(const QString& receiverId)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit friendRequestsLoaded(QJsonArray());
        return;
    }

    qDebug() << "Querying friend requests for receiver:" << receiverId;

    if (receiverId.isEmpty()) {
        emit friendRequestsLoaded(QJsonArray());
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();  // 使用 lanchat.db（本地存储）
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit friendRequestsLoaded(QJsonArray());
        return;
    }

    // 查询状态为 Pending (0) 的好友请求
    QSqlQuery q(db);
    q.prepare("SELECT requestId, senderId, receiverId, senderAccount, senderNickname, "
        "avatarPath, verifymsg, status, timestamp "
        "FROM friend_requests "
        "WHERE receiverId = :receiverId AND status = 0 "
        "ORDER BY timestamp DESC");
    q.bindValue(":receiverId", receiverId);

    if (!q.exec()) {
        qDebug() << "Query friend requests failed:" << q.lastError().text();
        emit errorOccurred("Query friend requests failed: " + q.lastError().text());
        emit friendRequestsLoaded(QJsonArray());
        return;
    }

    QJsonArray requests;
    while (q.next()) {
        QJsonObject request;
        request["requestId"] = q.value("requestId").toString();
        request["senderId"] = q.value("senderId").toString();
        request["receiverId"] = q.value("receiverId").toString();
        request["senderAccount"] = q.value("senderAccount").toString();
        request["senderNickname"] = q.value("senderNickname").toString();
        request["avatarPath"] = q.value("avatarPath").toString();
        request["verifymsg"] = q.value("verifymsg").toString();
        request["status"] = q.value("status").toInt();
        request["timestamp"] = q.value("timestamp").toLongLong();
        requests.append(request);
    }

    qDebug() << "Found" << requests.size() << "friend requests";
    emit friendRequestsLoaded(requests);
}

void DbLogicWorker::acceptFriendRequest(const QString& requestId, const QString& senderId, const QString& receiverId)
{
    if (!m_dbInitialized) {
        emit errorOccurred("Database not initialized");
        emit friendRequestAccepted(false, "数据库未初始化");
        return;
    }

    qDebug() << "Accepting friend request:" << requestId;

    if (requestId.isEmpty() || senderId.isEmpty() || receiverId.isEmpty()) {
        emit friendRequestAccepted(false, "请求ID或用户ID不能为空");
        return;
    }

    auto& dbm = DatabaseManager::getInstance();
    QSqlDatabase db = dbm.database();  // 使用 lanchat.db（本地存储）
    if (!db.isOpen()) {
        emit errorOccurred("Database not open");
        emit friendRequestAccepted(false, "数据库未打开");
        return;
    }

    // 开始事务
    if (!db.transaction()) {
        emit friendRequestAccepted(false, "开始事务失败");
        return;
    }

    // 1. 更新好友请求状态为 Accepted (1)
    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE friend_requests SET status = 1 WHERE requestId = :requestId");
    updateQuery.bindValue(":requestId", requestId);

    if (!updateQuery.exec()) {
        db.rollback();
        qDebug() << "Update friend request status failed:" << updateQuery.lastError().text();
        emit friendRequestAccepted(false, "更新请求状态失败: " + updateQuery.lastError().text());
        return;
    }

    // 2. 添加双向好友关系（双方都能看到对方）
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();

    // 添加 receiver -> sender 的好友关系
    QSqlQuery insertFriend1(db);
    insertFriend1.prepare("INSERT OR IGNORE INTO friends (userId, friendId, remark, addedTime) "
        "VALUES (:userId, :friendId, :remark, :addedTime)");
    insertFriend1.bindValue(":userId", receiverId);
    insertFriend1.bindValue(":friendId", senderId);
    insertFriend1.bindValue(":remark", QString());
    insertFriend1.bindValue(":addedTime", currentTime);

    if (!insertFriend1.exec()) {
        db.rollback();
        qDebug() << "Insert friend relationship 1 failed:" << insertFriend1.lastError().text();
        emit friendRequestAccepted(false, "添加好友关系失败: " + insertFriend1.lastError().text());
        return;
    }

    // 添加 sender -> receiver 的好友关系（可选，如果需要双向显示）
    QSqlQuery insertFriend2(db);
    insertFriend2.prepare("INSERT OR IGNORE INTO friends (userId, friendId, remark, addedTime) "
        "VALUES (:userId, :friendId, :remark, :addedTime)");
    insertFriend2.bindValue(":userId", senderId);
    insertFriend2.bindValue(":friendId", receiverId);
    insertFriend2.bindValue(":remark", QString());
    insertFriend2.bindValue(":addedTime", currentTime);

    if (!insertFriend2.exec()) {
        db.rollback();
        qDebug() << "Insert friend relationship 2 failed:" << insertFriend2.lastError().text();
        emit friendRequestAccepted(false, "添加好友关系失败: " + insertFriend2.lastError().text());
        return;
    }

    // 提交事务
    if (!db.commit()) {
        db.rollback();
        emit friendRequestAccepted(false, "提交事务失败");
        return;
    }

    qDebug() << "Friend request accepted successfully";
    emit friendRequestAccepted(true, QString());
}
