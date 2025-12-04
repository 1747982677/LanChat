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
    // 构造函数：初始化成员变量
    // 可以尝试从本地加载已保存的 Token
    m_token = loadTokenFromLocal();
    if (!m_token.isEmpty()) {
        // 验证本地 Token 是否有效
        if (validateToken(m_token)) {
            m_isLoggedIn = true;
            Logger::getInstance().log("Loaded token from local storage");
        } else {
            m_token.clear();
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
    
    // TODO: 实现注销逻辑
    // 1. 发送注销请求到服务器
    // 2. 清除本地 Token
    // 3. 重置登录状态
    
    m_currentUserId.clear();
    m_token.clear();
    m_isLoggedIn = false;
    
    // 清除本地保存的 Token
    // TODO: 实现清除本地 Token 的逻辑
    
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
    
    // TODO: 实现 Token 验证逻辑
    // 1. 检查 Token 格式
    // 2. 检查 Token 是否过期
    // 3. 可选：向服务器验证 Token 有效性
    
    // 临时实现：简单检查 Token 不为空
    return !token.isEmpty();
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
        
        // 注册成功后，自动保存账号密码到本地（在 Service 层内部处理）
        if (!m_pendingAccount.isEmpty() && !m_pendingPassword.isEmpty()) {
            saveCredentialsToLocal(m_pendingAccount, m_pendingPassword);
            Logger::getInstance().log("Saved credentials for registered account: " + m_pendingAccount);
            // 清除临时保存的密码（安全考虑）
            m_pendingPassword.clear();
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
    // TODO: 实现保存 Token 到本地
    // 可以使用 Config 或直接写入文件
    // Config::getInstance().setString("auth/token", token);
}

QString AuthService::loadTokenFromLocal() const
{
    // TODO: 实现从本地加载 Token
    // return Config::getInstance().getString("auth/token", QString());
    return QString();  // 临时返回空
}

void AuthService::saveCredentialsToLocal(const QString& account, const QString& password)
{
    // 保存账号和密码到本地（用于注册后自动填充）
    // TODO: 使用 Config 保存
    // Config::getInstance().setString("auth/last_account", account);
    // Config::getInstance().setString("auth/last_password", password);
    
    // 注意：密码应该加密存储，这里只是示例
    Logger::getInstance().log("Saved credentials for account: " + account);
}

void AuthService::loadCredentialsFromLocal(QString& account, QString& password) const
{
    // 从本地加载账号和密码
    // TODO: 使用 Config 加载
    // account = Config::getInstance().getString("auth/last_account", QString());
    // password = Config::getInstance().getString("auth/last_password", QString());
    
    account.clear();
    password.clear();
}

bool AuthService::hasSavedCredentials() const
{
    // 检查是否有保存的凭证
    QString account, password;
    const_cast<AuthService*>(this)->loadCredentialsFromLocal(account, password);
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
    // 获取保存的账号和密码（用于自动填充，仅在注册后使用）
    // 注意：此方法仅用于注册后自动填充，其他场景应使用 getSavedAccount()
    const_cast<AuthService*>(this)->loadCredentialsFromLocal(account, password);
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
        // 构造登录成功响应
        QJsonObject response;
        response["type"] = "login_success";
        response["userId"] = userId;
        // TODO: 生成实际的 Token（可能需要与服务器交互）
        response["token"] = "token_" + userId;  // 临时 Token
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

