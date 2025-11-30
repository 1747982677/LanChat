#ifndef REGISTER_WINDOW_H
#define REGISTER_WINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

// 前置声明
class QVBoxLayout;
class QHBoxLayout;

// 注册窗口类
// 职责：显示注册界面，处理用户输入，调用 AuthService 进行注册
class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget* parent = nullptr);
    ~RegisterWindow();

signals:
    // 注册成功信号（通知外部可以返回登录界面）
    void registerSucceeded(const QString& account, const QString& password);
    
    // 返回登录信号
    void returnToLogin();

private slots:
    // 处理注册按钮点击
    void onRegisterButtonClicked();
    
    // 处理注册成功
    void onRegisterSucceeded(const QString& account);
    
    // 处理注册失败
    void onRegisterFailed(const QString& errorMessage, const QString& errorType);
    
    // 处理协议复选框状态变化
    void onAgreementCheckBoxStateChanged(int state);
    
    // 处理返回登录链接点击
    void onReturnToLoginLinkClicked();
    
    // 处理账号输入变化（实时验证）
    void onAccountTextChanged(const QString& text);
    
    // 处理密码输入变化（实时验证）
    void onPasswordTextChanged(const QString& text);

private:
    // 初始化 UI
    void setupUi();
    
    // 设置样式
    void setupStyle();
    
    // 连接信号槽
    void connectSignals();
    
    // 验证邮箱格式
    bool validateEmail(const QString& email) const;
    
    // 验证密码格式（6-16位字母或数字）
    bool validatePassword(const QString& password) const;
    
    // 显示全局提示
    void showGlobalHint(const QString& message, bool isError = false);
    
    // 隐藏全局提示
    void hideGlobalHint();
    
    // 显示输入框提示文案
    void showAccountHint(const QString& message);
    void showPasswordHint(const QString& message);
    void hideAccountHint();
    void hidePasswordHint();
    
    // 成员变量
    QVBoxLayout* m_mainLayout;           // 主布局
    QLabel* m_titleLabel;               // 标题标签（欢迎注册+应用名）
    QLabel* m_globalHintLabel;          // 全局提示标签（顶部）
    QLineEdit* m_accountEdit;           // 账号输入框（邮箱）
    QLabel* m_accountHintLabel;         // 账号提示文案
    QLineEdit* m_passwordEdit;          // 密码输入框
    QLabel* m_passwordHintLabel;        // 密码提示文案
    QCheckBox* m_agreementCheckBox;     // 协议复选框
    QPushButton* m_registerButton;      // 注册按钮
    QLabel* m_returnToLoginLinkLabel;   // 返回登录链接
};

#endif // REGISTER_WINDOW_H

