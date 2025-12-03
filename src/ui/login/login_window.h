#ifndef LOGIN_WINDOW_H
#define LOGIN_WINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

// 前置声明
class QVBoxLayout;
class QHBoxLayout;
class RegisterWindow;

// 登录窗口类
// 职责：显示登录界面，处理用户输入，调用 AuthService 进行登录
class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();
    
    // 设置账号和密码（用于注册后自动填充）
    void setAccountAndPassword(const QString& account, const QString& password);

signals:
    // 登录成功信号（通知外部可以显示主窗口）
    void loginSucceeded();

private slots:
    // 处理登录按钮点击
    void onLoginButtonClicked();
    
    // 处理登录成功
    void onLoginSucceeded(const QString& userId, const QString& token);
    
    // 处理登录失败
    void onLoginFailed(const QString& errorMessage);
    
    // 处理协议复选框状态变化
    void onAgreementCheckBoxStateChanged(int state);
    
    // 处理注册账号链接点击
    void onRegisterLinkClicked();

private:
    // 初始化 UI
    void setupUi();
    
    // 设置样式
    void setupStyle();
    
    // 连接信号槽
    void connectSignals();
    
    // 验证邮箱格式
    bool validateEmail(const QString& email) const;
    
    // 显示全局提示
    void showGlobalHint(const QString& message, bool isError = false);
    
    // 隐藏全局提示
    void hideGlobalHint();
    
    // 加载保存的账号密码
    void loadSavedCredentials();
    
    // 成员变量
    QVBoxLayout* m_mainLayout;           // 主布局
    QLabel* m_titleLabel;               // 标题标签（Logo+应用名）
    QLabel* m_globalHintLabel;          // 全局提示标签（顶部）
    QLineEdit* m_accountEdit;           // 账号输入框（邮箱）
    QLineEdit* m_passwordEdit;          // 密码输入框
    QCheckBox* m_agreementCheckBox;     // 协议复选框
    QPushButton* m_loginButton;          // 登录按钮
    QLabel* m_registerLinkLabel;        // 注册账号链接
    
    RegisterWindow* m_registerWindow;   // 注册窗口（用于切换）
};

#endif // LOGIN_WINDOW_H

