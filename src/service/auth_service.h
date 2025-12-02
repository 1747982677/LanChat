#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <QObject>
#include <QString>
#include <QJsonObject>

namespace LanChat {
    struct UserInfo;
}

// 认证服务类
// 职责：处理登录、注销、Token 管理
class AuthService : public QObject
{
    Q_OBJECT

public:
    // 单例模式
    static AuthService& getInstance() {
        static AuthService instance;
        return instance;
    }

    // 登录（账号密码）
    void login(const QString& account, const QString& password);
    
    // 注册（账号密码）
    void registerAccount(const QString& account, const QString& password);
    
    // 注销
    void logout();
    
    // 检查是否已登录
    bool isLoggedIn() const;
    
    // 获取当前用户 ID
    QString getCurrentUserId() const;
    
    // 获取当前 Token
    QString getToken() const;
    
    // 验证 Token 是否有效
    bool validateToken(const QString& token) const;
    
    // 刷新 Token
    void refreshToken();
    
    // 检查是否有保存的凭证
    bool hasSavedCredentials() const;
    
    // 获取保存的账号（不返回密码，安全考虑）
    QString getSavedAccount() const;
    
    // 获取保存的账号和密码（用于自动填充，仅在注册后使用）
    // 注意：此方法仅用于注册后自动填充，其他场景应使用 getSavedAccount()
    void getSavedCredentials(QString& account, QString& password) const;

signals:
    // 登录成功信号
    void loginSucceeded(const QString& userId, const QString& token);
    
    // 登录失败信号
    void loginFailed(const QString& errorMessage);
    
    // 注册成功信号
    void registerSucceeded(const QString& account);
    
    // 注册失败信号（errorType: "account_exists", "password_invalid", "network_error"）
    void registerFailed(const QString& errorMessage, const QString& errorType);
    
    // 注销完成信号
    void logoutCompleted();
    
    // Token 过期信号
    void tokenExpired();

private:
    // 单例模式：私有构造函数
    explicit AuthService(QObject* parent = nullptr);
    ~AuthService() = default;
    
    // 禁止拷贝和赋值
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;
    
    // 处理登录响应
    void handleLoginResponse(const QJsonObject& response);
    
    // 处理注册响应
    void handleRegisterResponse(const QJsonObject& response);
    
    // 保存 Token 到本地（如果需要）
    void saveTokenToLocal(const QString& token);
    
    // 从本地加载 Token（如果需要）
    QString loadTokenFromLocal() const;
    
    // 保存账号密码到本地（用于注册后自动填充）
    void saveCredentialsToLocal(const QString& account, const QString& password);
    
    // 从本地加载账号密码
    void loadCredentialsFromLocal(QString& account, QString& password) const;
    
    // 成员变量
    QString m_currentUserId;      // 当前用户 ID
    QString m_token;              // 当前 Token
    bool m_isLoggedIn;            // 是否已登录
    
    // 临时保存注册时的账号密码（用于注册成功后保存）
    QString m_pendingAccount;     // 待保存的账号
    QString m_pendingPassword;    // 待保存的密码
};

#endif // AUTH_SERVICE_H

