#include "dblogic_controller.h"
#include "dblogic_worker.h"
#include "utils/password_util.h"
#include "ui/personinfo/UserEntity.h"
#include <QDebug>
#include <QUuid>
#include <QTimer>

DbLogicController* DbLogicController::s_instance = nullptr;

DbLogicController::DbLogicController(QObject* parent)
    : BaseController(parent)
    , m_dbInitialized(false)
    , m_dbPath()
{
}

DbLogicController::~DbLogicController()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

DbLogicController& DbLogicController::instance()
{
    if (!s_instance) {
        s_instance = new DbLogicController();
    }
    return *s_instance;
}

bool DbLogicController::isDatabaseInitialized() const
{
    return m_dbInitialized;
}

bool DbLogicController::initialize()
{
    QObject* worker = createWorker();
    if (!worker) {
        emit errorOccurred("Failed to create DbLogicWorker");
        return false;
    }

    setupWorkerThread(worker);
    connectSignals();
    QMetaObject::invokeMethod(worker, "initialize", Qt::QueuedConnection);

    qDebug() << "DbLogicController initialized";
    emit initialized();
    return true;
}

QObject* DbLogicController::createWorker()
{
    return new DbLogicWorker();
}

void DbLogicController::connectSignals()
{
    if (!m_worker) {
        return;
    }

    DbLogicWorker* worker = qobject_cast<DbLogicWorker*>(m_worker);
    if (!worker) {
        return;
    }

    // Controller -> Worker 信号
    connect(this, &DbLogicController::requestInitializeDatabase,
            worker, &DbLogicWorker::initializeDatabase);
    connect(this, &DbLogicController::requestSaveMessage,
            worker, &DbLogicWorker::saveMessage);
    connect(this, &DbLogicController::requestLoadHistoryMessages,
            worker, &DbLogicWorker::loadHistoryMessages);
    connect(this, &DbLogicController::requestSearchMessages,
            worker, &DbLogicWorker::searchMessages);
    connect(this, &DbLogicController::requestUpdateMessageStatus,
            worker, &DbLogicWorker::updateMessageStatus);
    connect(this, &DbLogicController::requestDeleteMessage,
            worker, &DbLogicWorker::deleteMessage);
    connect(this, &DbLogicController::requestLoadContactList,
            worker, &DbLogicWorker::loadContactList);
    connect(this, &DbLogicController::requestAddContact,
            worker, &DbLogicWorker::addContact);
    connect(this, &DbLogicController::requestUpdateContact,
            worker, &DbLogicWorker::updateContact);
    connect(this, &DbLogicController::requestProcessFile,
            worker, &DbLogicWorker::processFile);
    connect(this, &DbLogicController::requestSearchUserByAccount,
            worker, &DbLogicWorker::searchUserByAccount);
    connect(this, &DbLogicController::requestRegisterUser,
            worker, &DbLogicWorker::registerUser);
    connect(this, &DbLogicController::requestVerifyUserPassword,
            worker, &DbLogicWorker::verifyUserPassword);
    connect(this, &DbLogicController::requestSendFriendRequest,
            worker, &DbLogicWorker::sendFriendRequest);
    connect(this, &DbLogicController::requestQueryFriendRequests,
            worker, &DbLogicWorker::queryFriendRequests);
    connect(this, &DbLogicController::requestAcceptFriendRequest,
            worker, &DbLogicWorker::acceptFriendRequest);

    // Worker -> Controller 信号（转发）
    connect(worker, &DbLogicWorker::databaseInitialized,
            this, [this](bool success) {
                m_dbInitialized = success;
                qDebug() << "DbLogicController: Database initialization status:" << success;
                emit databaseInitialized(success);
            });
    connect(worker, &DbLogicWorker::messageSaved,
            this, &DbLogicController::messageSaved);
    connect(worker, &DbLogicWorker::historyMessagesLoaded,
            this, &DbLogicController::historyMessagesLoaded);
    connect(worker, &DbLogicWorker::searchResultsReady,
            this, &DbLogicController::searchResultsReady);
    connect(worker, &DbLogicWorker::messageStatusUpdated,
            this, &DbLogicController::messageStatusUpdated);
    connect(worker, &DbLogicWorker::contactListLoaded,
            this, &DbLogicController::contactListLoaded);
    connect(worker, &DbLogicWorker::contactOperationCompleted,
            this, &DbLogicController::contactOperationCompleted);
    connect(worker, &DbLogicWorker::fileProcessed,
            this, &DbLogicController::fileProcessed);
    connect(worker, &DbLogicWorker::errorOccurred,
            this, &DbLogicController::errorOccurred);
    connect(worker, &DbLogicWorker::userSearchResult,
            this, &DbLogicController::userSearchResult);
    connect(worker, &DbLogicWorker::userRegistered,
            this, &DbLogicController::userRegistered);
    connect(worker, &DbLogicWorker::passwordVerified,
            this, &DbLogicController::passwordVerified);
    connect(worker, &DbLogicWorker::friendRequestSent,
            this, &DbLogicController::friendRequestSent);
    connect(worker, &DbLogicWorker::friendRequestsLoaded,
            this, &DbLogicController::friendRequestsLoaded);
    connect(worker, &DbLogicWorker::friendRequestAccepted,
            this, &DbLogicController::friendRequestAccepted);

    //*********关于lanchat/messages数据表的设计********
    connect(this, &DbLogicController::requestQueryMessages,
        worker, &DbLogicWorker::queryMessages);
    connect(worker, &DbLogicWorker::queryResultsReady,
        this, &DbLogicController::queryResultsReady);

    //*********关于public/user数据表的设计********
    connect(this, &DbLogicController::requestQueryUser,
        worker, &DbLogicWorker::queryUser);
    connect(worker, &DbLogicWorker::queryUserReady,
        this, &DbLogicController::queryUserReady);

    connect(this, &DbLogicController::requesUpdateUser,
        worker, &DbLogicWorker::updateUser);
    connect(worker, &DbLogicWorker::updateUserReady,
        this, &DbLogicController::updateUserReady);

    connect(this, &DbLogicController::requesAddUser,
        worker, &DbLogicWorker::addUser);
    connect(worker, &DbLogicWorker::addUserReady,
        this, &DbLogicController::addUserReady);
}

void DbLogicController::queryMessages(const QString& localUser, const QString& peer, int limit)
{
    qDebug() << "DbLogicController: Request query messages:" << localUser << "to" << peer;
    emit requestQueryMessages(localUser, peer, limit);
}

void DbLogicController::initializeDatabase(const QString& dbPath)
{
    qDebug() << "DbLogicController: Request initialize database:" << dbPath;
    m_dbPath = dbPath;  // 保存数据库路径
    emit requestInitializeDatabase(dbPath);
}

void DbLogicController::saveMessage(const QJsonObject& message)
{
    emit requestSaveMessage(message);
}

// 修复：生成并返回 requestId
QString DbLogicController::loadHistoryMessages(const QString& contactId, int limit, int offset)
{
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "DbLogicController: Request load history for" << contactId << "requestId:" << requestId;
    emit requestLoadHistoryMessages(requestId, contactId, limit, offset);
    return requestId;
}

void DbLogicController::searchMessages(const QString& keyword)
{
    qDebug() << "DbLogicController: Request search messages:" << keyword;
    emit requestSearchMessages(keyword);
}

void DbLogicController::updateMessageStatus(const QString& messageId, const QString& status)
{
    emit requestUpdateMessageStatus(messageId, status);
}

void DbLogicController::deleteMessage(const QString& messageId)
{
    emit requestDeleteMessage(messageId);
}

void DbLogicController::loadContactList(const QString& userId)
{
    qDebug() << "DbLogicController: Request load contact list for user:" << userId;
    emit requestLoadContactList(userId);
}

void DbLogicController::addContact(const QJsonObject& contactInfo)
{
    emit requestAddContact(contactInfo);
}

void DbLogicController::updateContact(const QString& contactId, const QJsonObject& contactInfo)
{
    emit requestUpdateContact(contactId, contactInfo);
}

void DbLogicController::processFile(const QString& filePath, const QJsonObject& options)
{
    qDebug() << "DbLogicController: Request process file:" << filePath;
    emit requestProcessFile(filePath, options);
}

void DbLogicController::searchUserByAccount(const QString& account)
{
    qDebug() << "DbLogicController: Request search user by account:" << account;
    emit requestSearchUserByAccount(account);
}

void DbLogicController::registerUser(const QString& email, const QString& password)
{
    qDebug() << "DbLogicController: Request register user:" << email;
    
    // 检查数据库是否已初始化
    if (!m_dbInitialized) {
        qDebug() << "Database not initialized, waiting for initialization...";
        // 如果数据库未初始化，等待初始化完成
        // 使用一次性连接，等待初始化完成后再注册
        QMetaObject::Connection* conn = new QMetaObject::Connection();
        *conn = connect(this, &DbLogicController::databaseInitialized,
                       this, [this, email, password, conn](bool success) {
                           disconnect(*conn);
                           delete conn;
                           
                           if (success) {
                               // 数据库初始化成功，继续注册
                               qDebug() << "Database initialized, proceeding with registration";
                               QString passwordHash = PasswordUtil::hashPassword(password);
                               emit requestRegisterUser(email, passwordHash);
                           } else {
                               // 数据库初始化失败，通知 AuthService
                               qDebug() << "Database initialization failed";
                               emit userRegistered(false, QString(), "数据库初始化失败");
                           }
                       });
        
        // 如果初始化正在进行中，上面的连接会等待
        // 如果还没有开始初始化，触发初始化
        if (m_dbPath.isEmpty()) {
            // 如果数据库路径为空，说明还没有调用过 initializeDatabase
            // 尝试从 AppContext 获取路径，或者使用默认路径
            qDebug() << "Database path not set, using default path";
            m_dbPath = "lanchat.db";  // 使用默认路径
        }
        
        // 触发数据库初始化
        qDebug() << "Triggering database initialization with path:" << m_dbPath;
        emit requestInitializeDatabase(m_dbPath);
        
        // 设置超时，如果 5 秒后仍未初始化，返回错误
        QTimer::singleShot(5000, this, [this, conn]() {
            if (!m_dbInitialized) {
                qDebug() << "Database initialization timeout";
                disconnect(*conn);
                delete conn;
                emit userRegistered(false, QString(), "数据库初始化超时，请稍后重试");
            }
        });
        
        return;
    }
    
    // 数据库已初始化，直接注册
    // 在 Controller 层进行密码哈希（因为需要访问密码工具类）
    // 这样 Worker 层只需要处理哈希后的密码，更安全
    QString passwordHash = PasswordUtil::hashPassword(password);
    emit requestRegisterUser(email, passwordHash);
}

void DbLogicController::verifyUserPassword(const QString& email, const QString& password)
{
    qDebug() << "DbLogicController: Request verify password for:" << email;
    emit requestVerifyUserPassword(email, password);
}

void DbLogicController::queryUser(const UserEntity& localUser)
{
    qDebug() << "DbLogicController: Request query user:" << localUser.userId;
    emit requestQueryUser(localUser);
}

void DbLogicController::updateUser(const UserEntity& localUser)
{
    qDebug() << "DbLogicController: Request update user:" << localUser.userId;
    emit requesUpdateUser(localUser);
}

void DbLogicController::addUser(const UserEntity& localUser)
{
    qDebug() << "DbLogicController: Request add user:" << localUser.userId;
    emit requesAddUser(localUser);
}

void DbLogicController::sendFriendRequest(const QString& senderId, const QString& receiverId,
                                         const QString& senderAccount, const QString& senderNickname,
                                         const QString& avatarPath, const QString& verifymsg)
{
    qDebug() << "DbLogicController: Request send friend request from" << senderId << "to" << receiverId;
    emit requestSendFriendRequest(senderId, receiverId, senderAccount, senderNickname, avatarPath, verifymsg);
}

void DbLogicController::queryFriendRequests(const QString& receiverId)
{
    qDebug() << "DbLogicController: Request query friend requests for:" << receiverId;
    emit requestQueryFriendRequests(receiverId);
}

void DbLogicController::acceptFriendRequest(const QString& requestId, const QString& senderId, const QString& receiverId)
{
    qDebug() << "DbLogicController: Request accept friend request:" << requestId;
    emit requestAcceptFriendRequest(requestId, senderId, receiverId);
}
