#include "login_window.h"
#include "register_window.h"
#include "service/auth_service.h"
#include "utils/config.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_globalHintLabel(nullptr)
    , m_accountEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_agreementCheckBox(nullptr)
    , m_loginButton(nullptr)
    , m_registerLinkLabel(nullptr)
    , m_registerWindow(nullptr)
{
    setupUi();
    setupStyle();
    connectSignals();
    
    // 设置窗口属性
    setWindowTitle("LanChat - 登录");
    resize(400, 450);
    
    // 居中显示
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // 加载保存的账号密码
    loadSavedCredentials();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(40, 30, 40, 30);
    
    // 标题（Logo+应用名）
    m_titleLabel = new QLabel("LanChat", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_mainLayout->addWidget(m_titleLabel);
    
    // 全局提示区域（顶部，初始隐藏）
    m_globalHintLabel = new QLabel(this);
    m_globalHintLabel->setAlignment(Qt::AlignCenter);
    m_globalHintLabel->setWordWrap(true);
    m_globalHintLabel->setMinimumHeight(30);
    m_globalHintLabel->hide();
    m_mainLayout->addWidget(m_globalHintLabel);
    
    m_mainLayout->addStretch();
    
    // 表单布局
    QVBoxLayout* formLayout = new QVBoxLayout();
    formLayout->setSpacing(15);
    
    // 账号输入框（邮箱）
    m_accountEdit = new QLineEdit(this);
    m_accountEdit->setPlaceholderText("请输入邮箱账号");
    m_accountEdit->setMinimumHeight(40);
    formLayout->addWidget(m_accountEdit);
    
    // 密码输入框
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(40);
    formLayout->addWidget(m_passwordEdit);
    
    m_mainLayout->addLayout(formLayout);
    
    // 协议复选框
    m_agreementCheckBox = new QCheckBox("已阅读并同意服务协议", this);
    m_mainLayout->addWidget(m_agreementCheckBox);
    
    // 登录按钮（初始禁用）
    m_loginButton = new QPushButton("登录", this);
    m_loginButton->setMinimumHeight(45);
    m_loginButton->setEnabled(false);  // 初始禁用，需要勾选协议
    m_mainLayout->addWidget(m_loginButton);
    
    m_mainLayout->addStretch();
    
    // 注册账号链接（底部）
    QHBoxLayout* linkLayout = new QHBoxLayout();
    linkLayout->addStretch();
    m_registerLinkLabel = new QLabel("<a href=\"#\">注册账号</a>", this);
    m_registerLinkLabel->setOpenExternalLinks(false);
    linkLayout->addWidget(m_registerLinkLabel);
    linkLayout->addStretch();
    m_mainLayout->addLayout(linkLayout);
    
    setLayout(m_mainLayout);
}

void LoginWindow::setupStyle()
{
    // 设置窗口样式
    setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078d4;"
        "}"
        "QPushButton {"
        "    background-color: #0078d4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005a9e;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
        "QLabel#globalHint {"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QLabel#globalHintError {"
        "    background-color: #fff3cd;"
        "    color: #856404;"
        "    border: 1px solid #ffc107;"
        "}"
        "QLabel#globalHintInfo {"
        "    background-color: #d1ecf1;"
        "    color: #0c5460;"
        "    border: 1px solid #bee5eb;"
        "}"
    );
    
    // 设置全局提示标签的对象名（用于样式）
    m_globalHintLabel->setObjectName("globalHint");
}

void LoginWindow::connectSignals()
{
    // 连接登录按钮点击信号
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    
    // 连接回车键登录
    connect(m_accountEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginButtonClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginButtonClicked);
    
    // 连接协议复选框状态变化
    connect(m_agreementCheckBox, &QCheckBox::stateChanged, 
            this, &LoginWindow::onAgreementCheckBoxStateChanged);
    
    // 连接注册账号链接点击
    connect(m_registerLinkLabel, &QLabel::linkActivated, 
            this, &LoginWindow::onRegisterLinkClicked);
    
    // 连接 AuthService 的信号
    AuthService& authService = AuthService::getInstance();
    connect(&authService, &AuthService::loginSucceeded, 
            this, &LoginWindow::onLoginSucceeded);
    connect(&authService, &AuthService::loginFailed, 
            this, &LoginWindow::onLoginFailed);
}

void LoginWindow::onLoginButtonClicked()
{
    QString account = m_accountEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    // 隐藏之前的错误信息
    hideGlobalHint();
    
    // 验证输入
    if (account.isEmpty()) {
        showGlobalHint("请输入账号", true);
        return;
    }
    
    // 验证邮箱格式
    if (!validateEmail(account)) {
        showGlobalHint("请输入有效的邮箱地址", true);
        return;
    }
    
    if (password.isEmpty()) {
        showGlobalHint("请输入密码", true);
        return;
    }
    
    // 检查协议复选框
    if (!m_agreementCheckBox->isChecked()) {
        showGlobalHint("请先阅读并同意服务协议", true);
        return;
    }
    
    // 显示"登录中..."提示
    showGlobalHint("登录中...", false);
    
    // 禁用登录按钮（防止重复点击）
    m_loginButton->setEnabled(false);
    m_agreementCheckBox->setEnabled(false);
    
    // 调用 AuthService 进行登录
    AuthService::getInstance().login(account, password);
}

void LoginWindow::onLoginSucceeded(const QString& userId, const QString& token)
{
    Q_UNUSED(token);
    qDebug() << "LoginWindow::onLoginSucceeded 被调用，userId:" << userId;
    
    // 恢复登录按钮状态
    m_loginButton->setEnabled(true);
    m_agreementCheckBox->setEnabled(true);
    
    // 隐藏提示信息
    hideGlobalHint();
    
    // 发出登录成功信号
    qDebug() << "准备发出 LoginWindow::loginSucceeded 信号";
    emit loginSucceeded();
    qDebug() << "已发出 LoginWindow::loginSucceeded 信号";
}

void LoginWindow::onLoginFailed(const QString& errorMessage)
{
    // 恢复登录按钮状态
    m_loginButton->setEnabled(true);
    m_agreementCheckBox->setEnabled(true);
    
    // 在全局提示区域显示错误信息
    showGlobalHint(errorMessage, true);
}

void LoginWindow::onAgreementCheckBoxStateChanged(int state)
{
    // 根据协议复选框状态启用/禁用登录按钮
    bool isChecked = (state == Qt::Checked);
    m_loginButton->setEnabled(isChecked);
}

void LoginWindow::onRegisterLinkClicked()
{
    // 创建并显示注册窗口
    if (!m_registerWindow) {
        // 将 parent 设置为 nullptr，使注册窗口成为独立窗口
        // 这样登录窗口隐藏时，注册窗口不会受到影响
        m_registerWindow = new RegisterWindow(nullptr);
        
        // 设置窗口标志，使其成为独立窗口
        m_registerWindow->setWindowFlags(Qt::Window);
        
        // 连接注册成功信号
        connect(m_registerWindow, &RegisterWindow::registerSucceeded,
                this, [this](const QString& account, const QString& password) {
                    // 注册成功后，从 AuthService 获取临时暂存的账号密码并填充
                    // 注意：这是临时暂存，登录成功后会清除
                    setAccountAndPassword(account, password);
                    // 隐藏注册窗口，显示登录窗口
                    m_registerWindow->hide();
                    this->show();
                    // 显示提示信息
                    showGlobalHint("注册成功！账号密码已自动填充", false);
                });
        
        // 连接返回登录信号
        connect(m_registerWindow, &RegisterWindow::returnToLogin,
                this, [this]() {
                    m_registerWindow->hide();
                    this->show();
                });
    }
    
    // 隐藏登录窗口，显示注册窗口
    this->hide();
    m_registerWindow->show();
    m_registerWindow->raise();  // 确保窗口在最前面
    m_registerWindow->activateWindow();  // 激活窗口
}

void LoginWindow::setAccountAndPassword(const QString& account, const QString& password)
{
    m_accountEdit->setText(account);
    m_passwordEdit->setText(password);
}

bool LoginWindow::validateEmail(const QString& email) const
{
    // 简单的邮箱格式验证
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QRegularExpressionMatch match = emailRegex.match(email);
    return match.hasMatch();
}

void LoginWindow::showGlobalHint(const QString& message, bool isError)
{
    m_globalHintLabel->setText(message);
    if (isError) {
        m_globalHintLabel->setObjectName("globalHintError");
    } else {
        m_globalHintLabel->setObjectName("globalHintInfo");
    }
    m_globalHintLabel->setStyleSheet(styleSheet());  // 重新应用样式
    m_globalHintLabel->show();
}

void LoginWindow::hideGlobalHint()
{
    m_globalHintLabel->hide();
}

void LoginWindow::loadSavedCredentials()
{
    // 从 AuthService 加载保存的账号（只加载账号，不加载密码）
    QString account, password;
    AuthService::getInstance().getSavedCredentials(account, password);
    if (!account.isEmpty()) {
        m_accountEdit->setText(account);
        // 不填充密码，用户需要重新输入密码（安全考虑）
        m_passwordEdit->clear();
    }
}

