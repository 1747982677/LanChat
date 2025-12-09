#include "auth_service.h"
#include "common/types.h"
#include "common/global.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "network/socket_client.h"
#include "core/dblogic_controller.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QRegularExpression>
#include <QTimer>
#include <QApplication>

AuthService::AuthService(QObject* parent)
    : QObject(parent)
    , m_isLoggedIn(false)
{
    // 如果没有 parent，设置一个默认的 parent（QApplication）
    if (!parent && qApp) {
        setParent(qApp);
    }
    // 初始化 Config（如果还没有初始化）
    Config& config = Config::getInstance();
    config.load("config.ini");
    
    // 构造函数：初始化成员变量
    // 是否允许自动加载本地 token 可通过配置控制（默认关闭）
    if (isAutoLoginEnabled()) {
        m_token = loadTokenFromLocal();
        if (!m_token.isEmpty()) {
            // 验证本地 Token 是否有效
            if (validateToken(m_token)) {
                // 从 Token 中提取 userId（Token格式：userId_timestamp_hash）
                QStringList parts = m_token.split('_');
                if (parts.size() >= 2) {
                    m_currentUserId = parts[0];
                    m_isLoggedIn = true;
                    Logger::getInstance().log("Loaded token from local storage, userId: " + m_currentUserId);
                    qDebug() << "=== AuthService: Loaded userId from token:" << m_currentUserId << "===";
                } else {
                    // Token 格式不正确，清除
                    m_token.clear();
                    Logger::getInstance().warning("Invalid token format, clearing token");
                }
            } else {
                m_token.clear();
                Logger::getInstance().warning("Token validation failed, clearing token");
            }
        }
    }
}

void AuthService::login(const QString& account, const QString& password)
{
    if (account.isEmpty() || password.isEmpty()) {
        emit loginFailed("账号或密码不能为空");
        return;
    }
    
    Logger::getInstance().log("Attempting to login with account: " + account);
    
    // 临时保存登录时的账号密码（用于登录成功后保存到本地）
    m_pendingAccount = account;
    m_pendingPassword = password;
    
    // 连接 DbLogicController 的信号
    DbLogicController& dbController = DbLogicController::instance();
    connect(&dbController, &DbLogicController::passwordVerified,
            this, &AuthService::onPasswordVerified, Qt::UniqueConnection);
    
    // 调用 DbLogicController 进行密码验证
    dbController.verifyUserPassword(account, password);
}

void AuthService::logout()
{
    if (!m_isLoggedIn) {
        return;
    }
    
    Logger::getInstance().log("Logging out user: " + m_currentUserId);
    
    // 清除本地保存的 Token
    Config& config = Config::getInstance();
    config.setString("auth/token", QString());
    config.save();
    
    // 清除登录状态
    m_currentUserId.clear();
    m_token.clear();
    m_isLoggedIn = false;
    
    Logger::getInstance().log("User logged out, token cleared");
    emit logoutCompleted();
}

bool AuthService::isLoggedIn() const
{
    return m_isLoggedIn;
}

QString AuthService::getCurrentUserId() const
{
    return m_currentUserId;
}

QString AuthService::getToken() const
{
    return m_token;
}

bool AuthService::validateToken(const QString& token) const
{
    if (token.isEmpty()) {
        return false;
    }
    
    // Token 格式：userId_timestamp_hash
    // 例如：abc123_1701234567_a1b2c3d4e5f6...
    QStringList parts = token.split('_');
    if (parts.size() < 3) {
        Logger::getInstance().warning("Invalid token format: " + token);
        return false;
    }
    
    // 检查时间戳是否过期（7天有效期）
    bool ok;
    qint64 timestamp = parts[1].toLongLong(&ok);
    if (!ok) {
        Logger::getInstance().warning("Invalid timestamp in token: " + parts[1]);
        return false;
    }
    
    // 检查是否过期（7天 = 604800秒）
    const qint64 TOKEN_EXPIRY_SECONDS = 7 * 24 * 60 * 60;
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    if (currentTime - timestamp > TOKEN_EXPIRY_SECONDS) {
        Logger::getInstance().warning("Token expired: " + token);
        return false;
    }
    
    // 验证 hash（可选：验证 hash 是否正确）
    // 可添加更复杂的验证逻辑，比如验证 hash 是否匹配
    
    return true;
}

void AuthService::refreshToken()
{
    if (!m_isLoggedIn || m_token.isEmpty()) {
        Logger::getInstance().warning("Cannot refresh token: not logged in");
        return;
    }
    
    Logger::getInstance().log("Refreshing token");
    
    // TODO: 实现 Token 刷新逻辑
    // 1. 发送刷新 Token 请求
    // 2. 更新本地 Token
    // 3. 保存新 Token
}

void AuthService::handleLoginResponse(const QJsonObject& response)
{
    // 处理登录响应
    QString type = response["type"].toString();
    qDebug() << "handleLoginResponse 被调用，type:" << type;
    
    if (type == "login_success") {
        m_currentUserId = response["userId"].toString();
        m_token = response["token"].toString();
        m_isLoggedIn = true;
        
        // 保存 Token 到本地
        saveTokenToLocal(m_token);
        
        // 登录成功后，保存账号到本地（用于下次启动时自动填充，只保存账号，不保存密码）
        if (!m_pendingAccount.isEmpty()) {
            saveLoginCredentials(m_pendingAccount);
            Logger::getInstance().log("Saved login account for next startup: " + m_pendingAccount);
        }
        
        // 清除临时暂存数据（包括注册和登录时的临时数据）
        m_pendingAccount.clear();
        m_pendingPassword.clear();
        Logger::getInstance().log("Cleared temporary credentials");
        
        Logger::getInstance().log("Login succeeded for user: " + m_currentUserId);
        qDebug() << "准备发出 loginSucceeded 信号，userId:" << m_currentUserId;
        emit loginSucceeded(m_currentUserId, m_token);
        qDebug() << "已发出 loginSucceeded 信号";
    } else if (type == "login_failed") {
        QString errorMsg = response["message"].toString();
        QString errorType = response["error_type"].toString();
        
        // 根据错误类型设置错误消息
        if (errorType == "auth_failed") {
            errorMsg = "账号或密码输入有误，请核对后重试";
        } else if (errorType == "network_error") {
            errorMsg = "无法连接服务器，请检查网络设置或稍后重试";
        }
        
        Logger::getInstance().error("Login failed: " + errorMsg);
        emit loginFailed(errorMsg);
    }
}

void AuthService::registerAccount(const QString& account, const QString& password)
{
    if (account.isEmpty() || password.isEmpty()) {
        emit registerFailed("账号或密码不能为空", "input_error");
        return;
    }
    
    // 验证邮箱格式
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(account).hasMatch()) {
        emit registerFailed("请输入有效的邮箱地址", "password_invalid");
        return;
    }
    
    // 验证密码格式（6-16位字母或数字）
    if (password.length() < 6 || password.length() > 16) {
        emit registerFailed("密码格式错误，请使用 6-16 位字母或数字", "password_invalid");
        return;
    }
    QRegularExpression passwordRegex(R"(^[a-zA-Z0-9]+$)");
    if (!passwordRegex.match(password).hasMatch()) {
        emit registerFailed("密码格式错误，请使用 6-16 位字母或数字", "password_invalid");
        return;
    }
    
    Logger::getInstance().log("Attempting to register account: " + account);
    
    // 临时保存账号密码，用于注册成功后自动保存
    m_pendingAccount = account;
    m_pendingPassword = password;
    
    // 连接 DbLogicController 的信号
    DbLogicController& dbController = DbLogicController::instance();
    connect(&dbController, &DbLogicController::userRegistered,
            this, &AuthService::onUserRegistered, Qt::UniqueConnection);
    
    // 调用 DbLogicController 进行用户注册（密码哈希在 Controller 层完成）
    dbController.registerUser(account, password);
}

void AuthService::handleRegisterResponse(const QJsonObject& response)
{
    // 处理注册响应
    QString type = response["type"].toString();
    
    if (type == "register_success") {
        QString account = response["account"].toString();
        Logger::getInstance().log("Register succeeded for account: " + account);
        
        // 注册成功后，临时保存账号密码到内存（用于立即在登录窗口显示）
        // 注意：这是临时暂存，不保存到本地，登录成功后会清除
        if (!m_pendingAccount.isEmpty() && !m_pendingPassword.isEmpty()) {
            Logger::getInstance().log("Temporarily saved credentials for registered account: " + m_pendingAccount);
            // 保持 m_pendingAccount 和 m_pendingPassword 在内存中，供登录窗口使用
            // 登录成功后会清除这些临时数据
        }
        
        emit registerSucceeded(account);
    } else if (type == "register_failed") {
        QString errorMsg = response["message"].toString();
        QString errorType = response["error_type"].toString();
        
        // 根据错误类型设置错误消息
        if (errorType == "account_exists") {
            emit registerFailed("该账号已被注册，请尝试其他账号", "account_exists");
        } else if (errorType == "password_invalid") {
            emit registerFailed("密码格式错误，请使用 6-16 位字母或数字", "password_invalid");
        } else if (errorType == "database_error") {
            emit registerFailed(errorMsg.isEmpty() ? "数据库未初始化，请稍后重试" : errorMsg, "database_error");
        } else if (errorType == "network_error") {
            emit registerFailed("无法连接服务器，请稍后重试", "network_error");
        } else {
            emit registerFailed(errorMsg, errorType);
        }
        
        Logger::getInstance().error("Register failed: " + errorMsg);
    }
}

void AuthService::saveTokenToLocal(const QString& token)
{
    // 使用 Config 保存 Token 到本地
    Config& config = Config::getInstance();
    config.setString("auth/token", token);
    config.save();
    Logger::getInstance().log("Token saved to local storage");
}

QString AuthService::loadTokenFromLocal() const
{
    // 从本地加载 Token
    Config& config = Config::getInstance();
    QString token = config.getString("auth/token", QString());
    if (!token.isEmpty()) {
        Logger::getInstance().log("Token loaded from local storage");
    }
    return token;
}

bool AuthService::isAutoLoginEnabled() const
{
    const Config& config = Config::getInstance();
    return config.getString("auth/auto_login", "0") == "1";
}

void AuthService::saveCredentialsToLocal(const QString& account, const QString& password)
{
    // 这个方法已废弃，不再使用
    // 注册后的临时暂存不需要保存到本地，只保存在内存中
    Q_UNUSED(account);
    Q_UNUSED(password);
}

void AuthService::saveLoginCredentials(const QString& account)
{
    // 保存登录账号到本地（用于下次启动时自动填充，只保存账号，不保存密码）
    Config& config = Config::getInstance();
    config.setString("auth/last_account", account);
    // 不保存密码，提高安全性
    config.remove("auth/last_password"); // 清除可能存在的旧密码
    config.save();
    
    Logger::getInstance().log("Saved login account for next startup: " + account);
}

void AuthService::loadCredentialsFromLocal(QString& account, QString& password) const
{
    // 从本地加载账号（不加载密码，安全考虑）
    Config& config = Config::getInstance();
    account = config.getString("auth/last_account", QString());
    password = QString(); // 不加载密码，始终为空
    
    if (!account.isEmpty()) {
        Logger::getInstance().log("Loaded saved account: " + account);
    }
}

bool AuthService::hasSavedCredentials() const
{
    // 检查是否有保存的账号（不检查密码）
    QString account = getSavedAccount();
    return !account.isEmpty();
}

QString AuthService::getSavedAccount() const
{
    // 获取保存的账号（不返回密码，安全考虑）
    QString account, password;
    const_cast<AuthService*>(this)->loadCredentialsFromLocal(account, password);
    return account;
}

void AuthService::getSavedCredentials(QString& account, QString& password) const
{
    // 从本地配置文件加载保存的账号和密码（用于下次启动时自动填充）
    const_cast<AuthService*>(this)->loadCredentialsFromLocal(account, password);
}

void AuthService::getPendingCredentials(QString& account, QString& password) const
{
    // 获取注册后的临时暂存账号密码（用于注册成功后立即在登录窗口显示）
    account = m_pendingAccount;
    password = m_pendingPassword;
}

void AuthService::onUserRegistered(bool success, const QString& userId, const QString& errorMessage)
{
    if (success) {
        // 构造注册成功响应
        QJsonObject response;
        response["type"] = "register_success";
        response["account"] = m_pendingAccount;
        handleRegisterResponse(response);
    } else {
        // 构造注册失败响应
        QJsonObject response;
        response["type"] = "register_failed";
        response["message"] = errorMessage;
        
        // 根据错误消息内容设置正确的错误类型
        if (errorMessage.contains("已被注册") || errorMessage.contains("已存在")) {
            response["error_type"] = "account_exists";
        } else if (errorMessage.contains("数据库未初始化") || errorMessage.contains("Database not initialized")) {
            response["error_type"] = "database_error";
            response["message"] = "数据库未初始化，请稍后重试";
        } else if (errorMessage.contains("密码") || errorMessage.contains("password")) {
            response["error_type"] = "password_invalid";
        } else {
            response["error_type"] = "network_error";
        }
        handleRegisterResponse(response);
    }
}

void AuthService::onPasswordVerified(bool success, const QString& userId, const QString& errorMessage)
{
    if (success) {
        // 生成 Token：格式为 userId_timestamp_hash
        qint64 timestamp = QDateTime::currentSecsSinceEpoch();
        QString timestampStr = QString::number(timestamp);
        
        // 生成 hash：使用 userId + timestamp + 一个简单的密钥
        QString secret = "LanChat_Secret_Key_2024";  // 实际应用中应该使用更安全的密钥
        QString hashInput = userId + timestampStr + secret;
        QByteArray hashBytes = QCryptographicHash::hash(hashInput.toUtf8(), QCryptographicHash::Sha256);
        QString hash = hashBytes.toHex().left(16);  // 取前16位作为hash
        
        // 组合 Token：userId_timestamp_hash
        QString token = userId + "_" + timestampStr + "_" + hash;
        
        // 构造登录成功响应
        QJsonObject response;
        response["type"] = "login_success";
        response["userId"] = userId;
        response["token"] = token;
        handleLoginResponse(response);
    } else {
        // 构造登录失败响应
        QJsonObject response;
        response["type"] = "login_failed";
        response["message"] = errorMessage;
        response["error_type"] = "auth_failed";
        handleLoginResponse(response);
    }
}

