#include "db_manager.h"
#include "logger.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDir>
#include <QStandardPaths>

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() = default;

DatabaseManager::~DatabaseManager()
{
    // 不在析构中移除各线程连接，线程应显式调用 closeConnectionForCurrentThread()
}

QString DatabaseManager::defaultDbPath() const
{
    if (!m_dbPath.isEmpty()) return m_dbPath;
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return QDir(dir).filePath("lanchat.db");
}

QString DatabaseManager::connectionNameForCurrentThread() const
{
    quintptr id = reinterpret_cast<quintptr>(QThread::currentThreadId());
    return QString("lanchat_conn_%1").arg(id);
}

bool DatabaseManager::init(const QString& dbFilePath)
{
    if (!dbFilePath.isEmpty()) m_dbPath = dbFilePath;
    if (m_dbPath.isEmpty()) m_dbPath = defaultDbPath();

    QString conn = connectionNameForCurrentThread();
	Logger::getInstance().warning("Initializing database at: " + m_dbPath + " for connection: " + conn);
    if (!QSqlDatabase::contains(conn)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", conn);
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            Logger::getInstance().error("Database open failed: " + db.lastError().text());
            return false;
        }
    }
    else {
        QSqlDatabase db = QSqlDatabase::database(conn);
        if (!db.isOpen() && !db.open()) {
            Logger::getInstance().error("Database open failed (existing): " + db.lastError().text());
            return false;
        }
    }

    // 性能相关 PRAGMA
    if (!exec("PRAGMA journal_mode = WAL;")) {
        Logger::getInstance().warning("Failed to set WAL mode");
    }
    exec("PRAGMA synchronous = NORMAL;");

    return ensureSchema();
}

QSqlDatabase DatabaseManager::database()
{
    QString conn = connectionNameForCurrentThread();
    if (!QSqlDatabase::contains(conn)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", conn);
        db.setDatabaseName(m_dbPath.isEmpty() ? defaultDbPath() : m_dbPath);
        if (!db.open()) {
            Logger::getInstance().error("Database open failed for thread: " + db.lastError().text());
        }
        return db;
    }
    else {
        QSqlDatabase db = QSqlDatabase::database(conn);
        if (!db.isOpen()) {
            if (!db.open()) {
                Logger::getInstance().error("Database open failed for thread (existing): " + db.lastError().text());
            }
        }
        return db;
    }
}

bool DatabaseManager::exec(const QString& sql)
{
    QSqlDatabase db = database();
    if (!db.isOpen()) return false;
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        Logger::getInstance().error("SQL exec failed: " + q.lastError().text() + " SQL: " + sql);
        return false;
    }
    return true;
}

bool DatabaseManager::beginTransaction()
{
    QSqlDatabase db = database();
    if (!db.isOpen()) return false;
    return db.transaction();
}

bool DatabaseManager::commit()
{
    QSqlDatabase db = database();
    if (!db.isOpen()) return false;
    return db.commit();
}

bool DatabaseManager::rollback()
{
    QSqlDatabase db = database();
    if (!db.isOpen()) return false;
    return db.rollback();
}

void DatabaseManager::closeConnectionForCurrentThread()
{
    QString conn = connectionNameForCurrentThread();
    if (QSqlDatabase::contains(conn)) {
        QSqlDatabase db = QSqlDatabase::database(conn);
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(conn);
    }
}

bool DatabaseManager::ensureSchema()
{
    const QString createMessages = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            receiver TEXT NOT NULL,
            content TEXT NOT NULL,
            timestamp INTEGER NOT NULL,
            status INTEGER NOT NULL DEFAULT 0,
            extra TEXT
        );
    )";
    if (!exec(createMessages)) {
        Logger::getInstance().error("Failed to create messages table");
        return false;
    }
    exec("CREATE INDEX IF NOT EXISTS idx_messages_sender_receiver ON messages(sender, receiver);");

    // create chat_sessions table for unread counts
    const QString createSessions = R"(
        CREATE TABLE IF NOT EXISTS chat_sessions (
            userId TEXT PRIMARY KEY,
            nickname TEXT,
            avatarPath TEXT,
            lastMessage TEXT,
            lastTime INTEGER,
            unreadCount INTEGER DEFAULT 0
        );
    )";
    if (!exec(createSessions)) {
        Logger::getInstance().error("Failed to create chat_sessions table");
        return false;
    }

    // create users table for user information
    const QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            userId TEXT PRIMARY KEY,
            account TEXT UNIQUE NOT NULL,
            nickname TEXT,
            avatarPath TEXT,
            email TEXT,
            phone TEXT,
            signature TEXT,
            status INTEGER DEFAULT 0,
            lastOnlineTime INTEGER DEFAULT 0
        );
    )";
    if (!exec(createUsers)) {
        Logger::getInstance().error("Failed to create users table");
        return false;
    }
    exec("CREATE INDEX IF NOT EXISTS idx_users_account ON users(account);");

    // create friends table for friend relationships
    const QString createFriends = R"(
        CREATE TABLE IF NOT EXISTS friends (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId TEXT NOT NULL,
            friendId TEXT NOT NULL,
            remark TEXT,
            addedTime INTEGER NOT NULL,
            UNIQUE(userId, friendId)
        );
    )";
    if (!exec(createFriends)) {
        Logger::getInstance().error("Failed to create friends table");
        return false;
    }
    exec("CREATE INDEX IF NOT EXISTS idx_friends_userId ON friends(userId);");
    exec("CREATE INDEX IF NOT EXISTS idx_friends_friendId ON friends(friendId);");

    // create friend_requests table for friend requests
    const QString createFriendRequests = R"(
        CREATE TABLE IF NOT EXISTS friend_requests (
            requestId TEXT PRIMARY KEY,
            senderId TEXT NOT NULL,
            receiverId TEXT NOT NULL,
            senderAccount TEXT,
            senderNickname TEXT,
            avatarPath TEXT,
            verifymsg TEXT,
            status INTEGER DEFAULT 0,
            timestamp INTEGER NOT NULL
        );
    )";
    if (!exec(createFriendRequests)) {
        Logger::getInstance().error("Failed to create friend_requests table");
        return false;
    }
    exec("CREATE INDEX IF NOT EXISTS idx_friend_requests_receiverId ON friend_requests(receiverId);");
    exec("CREATE INDEX IF NOT EXISTS idx_friend_requests_status ON friend_requests(status);");

    return true;
}