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

//QString DatabaseManager::connectionNameForCurrentThread() const
//{
//    quintptr id = reinterpret_cast<quintptr>(QThread::currentThreadId());
//    return QString("lanchat_conn_%1").arg(id);
//}

bool DatabaseManager::init(const QString& dbFilePath)
{
    if (!dbFilePath.isEmpty()) m_dbPath = dbFilePath;
    if (m_dbPath.isEmpty()) m_dbPath = defaultDbPath();

    QString conn = connectionNameForCurrentThread("lanchat");
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
    m_connectionPaths["lanchat"] = dbFilePath;

    return ensureSchema("lanchat");
}

QSqlDatabase DatabaseManager::database()
{
    QString conn = connectionNameForCurrentThread("lanchat");
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
    QString conn = connectionNameForCurrentThread("lanchat");
    if (QSqlDatabase::contains(conn)) {
        QSqlDatabase db = QSqlDatabase::database(conn);
        if (db.isOpen()) db.close();
        QSqlDatabase::removeDatabase(conn);
    }
}

bool DatabaseManager::ensureSchema()
{
    return false;
}

bool DatabaseManager::initConnection(const QString& connectionName, const QString& dbFilePath)
{
    QMutexLocker locker(&m_mutex);

    // 验证连接名
    if (connectionName.isEmpty() || connectionName == m_defaultConnectionName) {
        qWarning() << "Invalid connection name";
        return false;
    }

    // 存储连接信息（在需要时创建连接）
    // 实际连接将在第一次访问时创建
    // 这里主要验证数据库文件是否可访问
    if (!dbFilePath.isEmpty() && dbFilePath != ":memory:") {
        QFileInfo fileInfo(dbFilePath);
        if (!fileInfo.dir().exists()) {
            return false;
        }
    }
    // 为默认连接注册路径
    m_connectionPaths[connectionName] = dbFilePath;
    ensureSchema(connectionName);
    Logger::getInstance().warning("Initializing database at: " + dbFilePath + " for connection: " + connectionName);
    return true;
}

bool DatabaseManager::ensureSchema(const QString& connectionName)
{
    if (connectionName=="public") {
        // create users table for user information
        const QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            userId TEXT PRIMARY KEY,
            nickname TEXT,
            avatarPath TEXT,
            email TEXT UNIQUE NOT NULL,
            phone TEXT,
            signature TEXT,
            status INTEGER DEFAULT 0,
            password TEXT NOT NULL,
            lastOnlineTime INTEGER DEFAULT 0
            );
        )";
        if (!exec(createUsers, connectionName)) {
            Logger::getInstance().error("Failed to create users table");
            return false;
        }
        exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);", connectionName);
        return true;
    }
    else if (connectionName == "lanchat") {
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
        if (!exec(createMessages, connectionName)) {
            Logger::getInstance().error("Failed to create messages table");
            return false;
        }
        exec("CREATE INDEX IF NOT EXISTS idx_messages_sender_receiver ON messages(sender, receiver);", connectionName);

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
        if (!exec(createSessions, connectionName)) {
            Logger::getInstance().error("Failed to create chat_sessions table");
            return false;
        }

        // create users table for user authentication (in lanchat database)
        const QString createUsers = R"(
            CREATE TABLE IF NOT EXISTS users (
                userId TEXT PRIMARY KEY,
                email TEXT UNIQUE NOT NULL,
                passwordHash TEXT NOT NULL,
                nickname TEXT,
                avatarPath TEXT,
                phone TEXT,
                signature TEXT,
                status INTEGER DEFAULT 0,
                lastOnlineTime INTEGER DEFAULT 0
            );
        )";
        if (!exec(createUsers, connectionName)) {
            Logger::getInstance().error("Failed to create users table");
            return false;
        }
        exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);", connectionName);

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
        if (!exec(createFriends, connectionName)) {
            Logger::getInstance().error("Failed to create friends table");
            return false;
        }
        exec("CREATE INDEX IF NOT EXISTS idx_friends_userId ON friends(userId);", connectionName);
        exec("CREATE INDEX IF NOT EXISTS idx_friends_friendId ON friends(friendId);", connectionName);

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
        if (!exec(createFriendRequests, connectionName)) {
            Logger::getInstance().error("Failed to create friend_requests table");
            return false;
        }
        exec("CREATE INDEX IF NOT EXISTS idx_friend_requests_receiverId ON friend_requests(receiverId);", connectionName);
        exec("CREATE INDEX IF NOT EXISTS idx_friend_requests_status ON friend_requests(status);", connectionName);

        return true;
    }
    else
        return false;
    
}

QSqlDatabase DatabaseManager::database(const QString& connectionName)
{
    QString name = connectionName.isEmpty() ? m_defaultConnectionName : connectionName;
    QString dbPath = (name == m_defaultConnectionName) ? m_dbPath : m_connectionPaths[connectionName];

    // 注意：这里需要你维护一个连接名到文件路径的映射
    // 简单实现：假设默认数据库是第一个 init() 调用的，其他通过 initConnection() 设置
    // 为了最小改动，你可能需要添加一个 QMap<QString, QString> m_connectionPaths 成员

    return getOrCreateConnection(name, dbPath);
}

// 内部方法：获取或创建连接
QSqlDatabase DatabaseManager::getOrCreateConnection(const QString& baseConnectionName, const QString& dbFilePath)
{
    QString threadConnectionName = connectionNameForCurrentThread(baseConnectionName);

    if (!QSqlDatabase::contains(threadConnectionName)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", threadConnectionName);
        db.setDatabaseName(dbFilePath.isEmpty() ? defaultDbPath() : dbFilePath);

        if (!db.open()) {
            qWarning() << "Failed to open database:" << db.lastError();
            return QSqlDatabase();
        }

        // 设置数据库参数
        db.exec("PRAGMA foreign_keys = ON");
        db.exec("PRAGMA journal_mode = WAL");
    }

    return QSqlDatabase::database(threadConnectionName);
}

QString DatabaseManager::connectionNameForCurrentThread(const QString& baseConnectionName) const
{
    QString name = baseConnectionName.isEmpty() ? m_defaultConnectionName : baseConnectionName;
    return QString("%1_%2").arg(name).arg((quintptr)QThread::currentThreadId());
}

// 执行指定连接名的 SQL
bool DatabaseManager::exec(const QString& sql, const QString& connectionName)
{
    QSqlDatabase db = database(connectionName);
    if (!db.isValid() || !db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    return query.exec(sql);
}